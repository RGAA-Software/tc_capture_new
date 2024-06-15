//
// Created by RGAA on 2023/8/20.
//
#include "cursor_capture.h"
#include <Windows.h>
#include <iostream>
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/data.h"
#include "capture_message.h"
#include "tc_common_new/log.h"

namespace tc
{

    static uint8_t *get_bitmap_data(HBITMAP hbmp, BITMAP *bmp, uint32_t *sizeOut) {
        if (GetObject(hbmp, sizeof(*bmp), bmp) != 0) {
            uint8_t *output;
            unsigned int size =
                    (bmp->bmHeight * bmp->bmWidth * bmp->bmBitsPixel) / 8;
            if (sizeOut) {
                *sizeOut = size;
            }
            output = (uint8_t *) malloc(size);
            GetBitmapBits(hbmp, size, output);
            return output;
        }

        return nullptr;
    }

    static inline uint8_t bit_to_alpha(uint8_t *data, long pixel, bool invert) {
        uint8_t pix_byte = data[pixel / 8];
        bool alpha = (pix_byte >> (7 - pixel % 8) & 1) != 0;

        if (invert) {
            return alpha ? 0xFF : 0;
        } else {
            return alpha ? 0 : 0xFF;
        }
    }

    static inline bool bitmap_has_alpha(uint8_t *data, long num_pixels) {
        for (long i = 0; i < num_pixels; i++) {
            if (data[i * 4 + 3] != 0) {
                return true;
            }
        }

        return false;
    }

    static inline void apply_mask(uint8_t *color, uint8_t *mask, long num_pixels) {
        for (long i = 0; i < num_pixels; i++)
            color[i * 4 + 3] = bit_to_alpha(mask, i, false);
    }

    static inline void apply_mask(uint8_t *color, uint8_t *mask, BITMAP *bmp_mask) {
        long mask_pix_offs;

        for (long y = 0; y < bmp_mask->bmHeight; y++) {
            for (long x = 0; x < bmp_mask->bmWidth; x++) {
                mask_pix_offs = y * (bmp_mask->bmWidthBytes * 8) + x;
                color[(y * bmp_mask->bmWidth + x) * 4 + 3] =
                        bit_to_alpha(mask, mask_pix_offs, false);
            }
        }
    }

    static inline uint8_t *copy_from_color(ICONINFO *ii, uint32_t *width,
                                           uint32_t *height, uint32_t *sizeOut) {
        BITMAP bmp_color;
        BITMAP bmp_mask;
        uint8_t *color;
        uint8_t *mask;

        color = get_bitmap_data(ii->hbmColor, &bmp_color, sizeOut);
        if (!color) {
            return nullptr;
        }

        if (bmp_color.bmBitsPixel < 32) {
            free(color);
            return nullptr;
        }

        mask = get_bitmap_data(ii->hbmMask, &bmp_mask, nullptr);
        if (mask) {
            long pixels = bmp_color.bmHeight * bmp_color.bmWidth;

            if (!bitmap_has_alpha(color, pixels)) {
                //apply_mask(color, mask, pixels);
                apply_mask(color, mask, &bmp_mask); //修复编辑框内，鼠标颜色不显示的问题
            }
            free(mask);
        }

        *width = bmp_color.bmWidth;
        *height = bmp_color.bmHeight;
        return color;
    }

    static inline uint8_t *copy_from_mask(ICONINFO *ii, uint32_t *width,
                                          uint32_t *height, uint32_t *sizeOut) {
        uint8_t *output;
        uint8_t *mask;
        long pixels;
        long bottom;
        BITMAP bmp;

        mask = get_bitmap_data(ii->hbmMask, &bmp, sizeOut);
        if (!mask) {
            return nullptr;
        }

        bmp.bmHeight /= 2;

        pixels = bmp.bmHeight * bmp.bmWidth;
        int outputSize = pixels * 4;
        if (sizeOut)
            *sizeOut = outputSize;
        output = (uint8_t *) calloc(1, outputSize);

        bottom = bmp.bmWidthBytes * bmp.bmHeight;

        for (long i = 0; i < pixels; i++) {
            uint8_t alpha = bit_to_alpha(mask, i, false);
            uint8_t color = bit_to_alpha(mask + bottom, i, true);
            if (!alpha) {
                output[i * 4 + 3] = color;
            } else {
                *(uint32_t *) &output[i * 4] = !!color ? 0xFFFFFFFF
                                                       : 0xFF000000;
            }
        }

        free(mask);

        *width = bmp.bmWidth;
        *height = bmp.bmHeight;
        return output;
    }

    static inline uint8_t *cursor_capture_icon_bitmap(ICONINFO *ii, uint32_t *width,
                                                      uint32_t *height, uint32_t *sizeOut) {
        uint8_t *output;

        output = copy_from_color(ii, width, height, sizeOut);
        if (!output) {
            output = copy_from_mask(ii, width, height, sizeOut);
        }
        return output;
    }

    static void reshape_image_rgba_order(CaptureCursorBitmap *cursor) {
        int offset = 0;
        for (int row = 0; row < cursor->height_; ++row) {
            for (int col = 0; col < cursor->width_; ++col) {
                char r = cursor->data_->At(offset);
                *((char *) cursor->data_->DataAddr() + offset) = *(cursor->data_->DataAddr() + offset + 2);
                *((char *) cursor->data_->DataAddr() + offset + 2) = r;
                offset += 4;
            }
        }
    }

    CursorCapture::CursorCapture(const std::shared_ptr<MessageNotifier> &msg_notifier) {
        msg_notifier_ = msg_notifier;
    }

    bool CursorCapture::CursorCaptureIcon(CaptureCursorBitmap *data, HICON icon) {
        uint8_t *bitmap;
        uint32_t height;
        uint32_t width;
        ICONINFO ii;

        if (!icon) {
            return false;
        }
        if (!GetIconInfo(icon, &ii)) {
            return false;
        }
        uint32_t bitmapSize = 0;
        bitmap = cursor_capture_icon_bitmap(&ii, &width, &height, &bitmapSize);
        if (bitmap) {
            data->data_ = Data::From(std::string(bitmap, bitmap + bitmapSize));
            data->width_ = width;
            data->height_ = height;
            data->hotspot_x_ = ii.xHotspot;
            data->hotspot_y_ = ii.yHotspot;
            free(bitmap);
        } else {
            return false;
        }
        DeleteObject(ii.hbmColor);
        DeleteObject(ii.hbmMask);
        return true;
    }

    void CursorCapture::Capture() {
        CaptureCursorBitmap cursor_bitmap;
        CURSORINFO ci = {0};
        HICON icon;
        ci.cbSize = sizeof(ci);

        if (!GetCursorInfo(&ci)) {
            cursor_bitmap.visable_ = true;
            return;
        }
        cursor_bitmap.visable_ = (ci.flags & CURSOR_SHOWING) == CURSOR_SHOWING;
        cursor_bitmap.x_ = ci.ptScreenPos.x;
        cursor_bitmap.y_ = ci.ptScreenPos.y;

        // RGB Data
        icon = CopyIcon(ci.hCursor);
        if (CursorCaptureIcon(&cursor_bitmap, icon)) {
            reshape_image_rgba_order(&cursor_bitmap);
        } else {
            return;
        }
        DestroyIcon(icon);
        if (msg_notifier_) {
            msg_notifier_->SendAppMessage(cursor_bitmap);
        }
#if 0   // save rgba to file
        static int index = 0;
        if(index > 20) {
            return;
        }
        auto rgba_file_name = std::format("test_cursor_{}X{}_{}_.rgba", cursor_bitmap.width_, cursor_bitmap.height_, ++index);
        auto file_ptr = File::OpenForWriteB(rgba_file_name);
        file_ptr->Write(0, cursor_bitmap.data_->DataAddr(), cursor_bitmap.data_->Size());
#endif
    }
}

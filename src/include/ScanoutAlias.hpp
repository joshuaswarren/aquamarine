#pragma once

#include <aquamarine/backend/Misc.hpp>
#include <drm_fourcc.h>
#include <vector>

namespace Aquamarine {
    // Alpha variant of an opaque 8 bpc format, or DRM_FORMAT_INVALID.
    inline uint32_t opaqueToAlphaFormat(uint32_t fmt) {
        if (fmt == DRM_FORMAT_XRGB8888)
            return DRM_FORMAT_ARGB8888;
        if (fmt == DRM_FORMAT_XBGR8888)
            return DRM_FORMAT_ABGR8888;
        return DRM_FORMAT_INVALID;
    }

    inline bool planeHasFormat(const std::vector<SDRMFormat>& planeFormats, uint32_t fmt) {
        if (fmt == DRM_FORMAT_INVALID)
            return false;
        for (auto const& f : planeFormats) {
            if (f.drmFormat == fmt)
                return true;
        }
        return false;
    }

    // The alpha variant to substitute for `requested` on this plane, or DRM_FORMAT_INVALID.
    // Eligibility is shared by the allocator and the commit-time checks: scanout only, never
    // cursor, and only when the plane lacks the opaque format but offers the alpha variant.
    // A plane that offers both formats must keep its original behavior.
    inline uint32_t scanoutAlphaAlias(const std::vector<SDRMFormat>& planeFormats, uint32_t requested, bool scanout, bool cursor) {
        const uint32_t alias = opaqueToAlphaFormat(requested);
        if (!scanout || cursor)
            return DRM_FORMAT_INVALID;
        if (planeHasFormat(planeFormats, requested))
            return DRM_FORMAT_INVALID;
        if (!planeHasFormat(planeFormats, alias))
            return DRM_FORMAT_INVALID;
        return alias;
    }

    // True when `bufferFormat` is exactly the eligible alias of `requested` on this plane.
    inline bool scanoutAlphaAliasAccepted(const std::vector<SDRMFormat>& planeFormats, uint32_t requested, uint32_t bufferFormat, bool scanout, bool cursor) {
        return bufferFormat != requested && bufferFormat == scanoutAlphaAlias(planeFormats, requested, scanout, cursor);
    }
}

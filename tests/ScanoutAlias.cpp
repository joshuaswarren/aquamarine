#include <cstdio>
#include <cstdlib>
#include <vector>
#include "ScanoutAlias.hpp"

using namespace Aquamarine;

// Survives -DNDEBUG, unlike assert().
#define CHECK(cond)                                                                                                                                            \
    do {                                                                                                                                                       \
        if (!(cond)) {                                                                                                                                         \
            std::fprintf(stderr, "scanoutAlias: FAILED %s at line %d\n", #cond, __LINE__);                                                                      \
            std::exit(1);                                                                                                                                       \
        }                                                                                                                                                      \
    } while (0)

static std::vector<SDRMFormat> plane(std::initializer_list<uint32_t> fmts) {
    std::vector<SDRMFormat> v;
    for (auto f : fmts)
        v.push_back(SDRMFormat{.drmFormat = f});
    return v;
}

int main() {
    const auto ALPHA_ONLY  = plane({DRM_FORMAT_ARGB8888, DRM_FORMAT_ABGR8888});
    const auto BOTH        = plane({DRM_FORMAT_XRGB8888, DRM_FORMAT_ARGB8888});
    const auto OPAQUE_ONLY = plane({DRM_FORMAT_XRGB8888});

    // Alpha-only plane: the fallback applies.
    CHECK(scanoutAlphaAlias(ALPHA_ONLY, DRM_FORMAT_XRGB8888, true, false) == DRM_FORMAT_ARGB8888);
    CHECK(scanoutAlphaAliasAccepted(ALPHA_ONLY, DRM_FORMAT_XRGB8888, DRM_FORMAT_ARGB8888, true, false));
    CHECK(!scanoutAlphaAliasAccepted(ALPHA_ONLY, DRM_FORMAT_XRGB8888, DRM_FORMAT_XRGB8888, true, false));

    // Both-format plane: no alias. An AR24 buffer next to an XR24 state is a real mismatch,
    // and the old mismatch/async handling must apply as before.
    CHECK(scanoutAlphaAlias(BOTH, DRM_FORMAT_XRGB8888, true, false) == DRM_FORMAT_INVALID);
    CHECK(!scanoutAlphaAliasAccepted(BOTH, DRM_FORMAT_XRGB8888, DRM_FORMAT_ARGB8888, true, false));

    // Opaque-only plane: nothing to fall back to.
    CHECK(scanoutAlphaAlias(OPAQUE_ONLY, DRM_FORMAT_XRGB8888, true, false) == DRM_FORMAT_INVALID);

    // Controls: cursor buffers and render-only allocations never alias.
    CHECK(scanoutAlphaAlias(ALPHA_ONLY, DRM_FORMAT_XRGB8888, true, true) == DRM_FORMAT_INVALID);
    CHECK(scanoutAlphaAlias(ALPHA_ONLY, DRM_FORMAT_XRGB8888, false, false) == DRM_FORMAT_INVALID);

    // An already-alpha request has no alias.
    CHECK(scanoutAlphaAlias(ALPHA_ONLY, DRM_FORMAT_ARGB8888, true, false) == DRM_FORMAT_INVALID);
    CHECK(opaqueToAlphaFormat(DRM_FORMAT_ARGB8888) == DRM_FORMAT_INVALID);

    std::puts("scanoutAlias: all checks passed");
    return 0;
}

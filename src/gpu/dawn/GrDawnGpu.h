/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnGpu_DEFINED
#define GrDawnGpu_DEFINED

#include "src/gpu/GrGpu.h"
#include "dawn/dawncpp.h"
#include "src/gpu/dawn/GrDawnRingBuffer.h"

class GrDawnOpsRenderPass;
class GrPipeline;

namespace SkSL {
    class Compiler;
}

class GrDawnGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(const dawn::Device& device, const GrContextOptions&, GrContext*);
    GrDawnGpu(GrContext* context, const GrContextOptions& options, const dawn::Device& device);

    ~GrDawnGpu() override;

    void disconnect(DisconnectType) override;

    const dawn::Device& device() const { return fDevice; }
    const dawn::Queue&  queue() const { return fQueue; }

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    GrBackendTexture createBackendTexture(int w, int h,
                                          const GrBackendFormat &,
                                          GrMipMapped,
                                          GrRenderable,
                                          const void* pixels,
                                          size_t rowBytes,
                                          const SkColor4f* color,
                                          GrProtected isProtected) override;
    void deleteBackendTexture(const GrBackendTexture&) override;
#if GR_TEST_UTILS
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(int w, int h, GrColorType) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void testingOnly_flushGpuAndSync() override;
#endif

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height,
                                                                int numStencilSamples) override;

    GrOpsRenderPass* getOpsRenderPass(
            GrRenderTarget*, GrSurfaceOrigin, const SkRect& bounds,
            const GrOpsRenderPass::LoadAndStoreInfo&,
            const GrOpsRenderPass::StencilLoadAndStoreInfo&) override;

    SkSL::Compiler* shaderCompiler() const {
        return fCompiler.get();
    }

    void submit(GrOpsRenderPass*) override;

    GrFence SK_WARN_UNUSED_RESULT insertFence() override;
    bool waitFence(GrFence, uint64_t timeout) override;
    void deleteFence(GrFence) const override;

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned = true) override;
    sk_sp<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                            GrResourceProvider::SemaphoreWrapType wrapType,
                                            GrWrapOwnership ownership) override;
    void insertSemaphore(sk_sp<GrSemaphore> semaphore) override;
    void waitSemaphore(sk_sp<GrSemaphore> semaphore) override;
    void checkFinishProcs() override;

    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override;

    GrDawnRingBuffer::Slice allocateUniformRingBufferSlice(int size);

private:
    void onResetContext(uint32_t resetBits) override {}

    virtual void querySampleLocations(GrRenderTarget*, SkTArray<SkPoint>*) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc,
                                     const GrBackendFormat&,
                                     GrRenderable,
                                     int renderTargetSampleCnt,
                                     SkBudgeted,
                                     GrProtected,
                                     const GrMipLevel texels[],
                                     int mipLevelCount) override;

    sk_sp<GrTexture> onCreateCompressedTexture(int width, int height, const GrBackendFormat&,
                                               SkImage::CompressionType, SkBudgeted,
                                               const void* data) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrColorType, GrWrapOwnership,
                                          GrWrapCacheable, GrIOType) override;
    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&, int sampleCnt,
                                                    GrColorType, GrWrapOwnership,
                                                    GrWrapCacheable) override;
    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&,
                                                    GrColorType) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&, int sampleCnt,
                                                             GrColorType) override;

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t size, GrGpuBufferType type, GrAccessPattern,
                                      const void* data) override;

    bool onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                      GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                      size_t rowBytes) override;

    bool onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                       GrColorType surfaceColorType, GrColorType dstColorType,
                       const GrMipLevel texels[], int mipLevelCount) override;

    bool onTransferPixelsTo(GrTexture*, int left, int top, int width, int height,
                            GrColorType textureColorType, GrColorType bufferColorType,
                            GrGpuBuffer* transferBuffer, size_t offset, size_t rowBytes) override;

    bool onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                              GrColorType surfaceColorType, GrColorType bufferColorType,
                              GrGpuBuffer* transferBuffer, size_t offset) override;

    void onResolveRenderTarget(GrRenderTarget* target) override {
    }

    bool onRegenerateMipMapLevels(GrTexture*) override;

    bool onCopySurface(GrSurface* dst, GrSurface* src,
                       const SkIRect& srcRect, const SkIPoint& dstPoint) override;

    void onFinishFlush(GrSurfaceProxy*[], int n, SkSurface::BackendSurfaceAccess access,
                       const GrFlushInfo& info, const GrPrepareForExternalIORequests&) override;

    dawn::Device                                    fDevice;
    dawn::Queue                                     fQueue;
    std::unique_ptr<SkSL::Compiler>                 fCompiler;
    std::unique_ptr<GrDawnOpsRenderPass>            fOpsRenderPass;
    GrDawnRingBuffer                                fUniformRingBuffer;

    typedef GrGpu INHERITED;
};

#endif

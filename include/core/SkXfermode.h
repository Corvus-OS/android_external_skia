
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkXfermode_DEFINED
#define SkXfermode_DEFINED

#include "SkFlattenable.h"
#include "SkColor.h"

class GrFragmentProcessor;
class GrTexture;
class GrXPFactory;
class SkString;

struct SkPM4f;
typedef SkPM4f (*SkXfermodeProc4f)(const SkPM4f& src, const SkPM4f& dst);

/** \class SkXfermode
 *
 *  SkXfermode is the base class for objects that are called to implement custom
 *  "transfer-modes" in the drawing pipeline. The static function Create(Modes)
 *  can be called to return an instance of any of the predefined subclasses as
 *  specified in the Modes enum. When an SkXfermode is assigned to an SkPaint,
 *  then objects drawn with that paint have the xfermode applied.
 *
 *  All subclasses are required to be reentrant-safe : it must be legal to share
 *  the same instance between several threads.
 */
class SK_API SkXfermode : public SkFlattenable {
public:
    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const;
    virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const;
    virtual void xferA8(SkAlpha dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const;

    /** Enum of possible coefficients to describe some xfermodes
     */
    enum Coeff {
        kZero_Coeff,    /** 0 */
        kOne_Coeff,     /** 1 */
        kSC_Coeff,      /** src color */
        kISC_Coeff,     /** inverse src color (i.e. 1 - sc) */
        kDC_Coeff,      /** dst color */
        kIDC_Coeff,     /** inverse dst color (i.e. 1 - dc) */
        kSA_Coeff,      /** src alpha */
        kISA_Coeff,     /** inverse src alpha (i.e. 1 - sa) */
        kDA_Coeff,      /** dst alpha */
        kIDA_Coeff,     /** inverse dst alpha (i.e. 1 - da) */

        kCoeffCount
    };

    /** List of predefined xfermodes.
        The algebra for the modes uses the following symbols:
        Sa, Sc  - source alpha and color
        Da, Dc - destination alpha and color (before compositing)
        [a, c] - Resulting (alpha, color) values
        For these equations, the colors are in premultiplied state.
        If no xfermode is specified, kSrcOver is assumed.
        The modes are ordered by those that can be expressed as a pair of Coeffs, followed by those
        that aren't Coeffs but have separable r,g,b computations, and finally
        those that are not separable.
     */
    enum Mode {
        kClear_Mode,    //!< [0, 0]
        kSrc_Mode,      //!< [Sa, Sc]
        kDst_Mode,      //!< [Da, Dc]
        kSrcOver_Mode,  //!< [Sa + Da * (1 - Sa), Sc + Dc * (1 - Sa)]
        kDstOver_Mode,  //!< [Da + Sa * (1 - Da), Dc + Sc * (1 - Da)]
        kSrcIn_Mode,    //!< [Sa * Da, Sc * Da]
        kDstIn_Mode,    //!< [Da * Sa, Dc * Sa]
        kSrcOut_Mode,   //!< [Sa * (1 - Da), Sc * (1 - Da)]
        kDstOut_Mode,   //!< [Da * (1 - Sa), Dc * (1 - Sa)]
        kSrcATop_Mode,  //!< [Da, Sc * Da + Dc * (1 - Sa)]
        kDstATop_Mode,  //!< [Sa, Dc * Sa + Sc * (1 - Da)]
        kXor_Mode,      //!< [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa)]
        kPlus_Mode,     //!< [Sa + Da, Sc + Dc]
        kModulate_Mode, // multiplies all components (= alpha and color)

        // Following blend modes are defined in the CSS Compositing standard:
        // https://dvcs.w3.org/hg/FXTF/rawfile/tip/compositing/index.html#blending
        kScreen_Mode,
        kLastCoeffMode = kScreen_Mode,

        kOverlay_Mode,
        kDarken_Mode,
        kLighten_Mode,
        kColorDodge_Mode,
        kColorBurn_Mode,
        kHardLight_Mode,
        kSoftLight_Mode,
        kDifference_Mode,
        kExclusion_Mode,
        kMultiply_Mode,
        kLastSeparableMode = kMultiply_Mode,

        kHue_Mode,
        kSaturation_Mode,
        kColor_Mode,
        kLuminosity_Mode,
        kLastMode = kLuminosity_Mode
    };

    /**
     * Gets the name of the Mode as a string.
     */
    static const char* ModeName(Mode);

    /**
     *  If the xfermode is one of the modes in the Mode enum, then asMode()
     *  returns true and sets (if not null) mode accordingly. Otherwise it
     *  returns false and ignores the mode parameter.
     */
    virtual bool asMode(Mode* mode) const;

    /**
     *  The same as calling xfermode->asMode(mode), except that this also checks
     *  if the xfermode is NULL, and if so, treats it as kSrcOver_Mode.
     */
    static bool AsMode(const SkXfermode*, Mode* mode);

    /**
     *  Returns true if the xfermode claims to be the specified Mode. This works
     *  correctly even if the xfermode is NULL (which equates to kSrcOver.) Thus
     *  you can say this without checking for a null...
     *
     *  If (SkXfermode::IsMode(paint.getXfermode(),
     *                         SkXfermode::kDstOver_Mode)) {
     *      ...
     *  }
     */
    static bool IsMode(const SkXfermode* xfer, Mode mode);

    /** Return an SkXfermode object for the specified mode.
     */
    static SkXfermode* Create(Mode mode);

    /** Return a function pointer to a routine that applies the specified
        porter-duff transfer mode.
     */
    static SkXfermodeProc GetProc(Mode mode);
    static SkXfermodeProc4f GetProc4f(Mode);

    virtual SkXfermodeProc4f getProc4f() const;

    /**
     *  If the specified mode can be represented by a pair of Coeff, then return
     *  true and set (if not NULL) the corresponding coeffs. If the mode is
     *  not representable as a pair of Coeffs, return false and ignore the
     *  src and dst parameters.
     */
    static bool ModeAsCoeff(Mode mode, Coeff* src, Coeff* dst);

    SK_ATTR_DEPRECATED("use AsMode(...)")
    static bool IsMode(const SkXfermode* xfer, Mode* mode) {
        return AsMode(xfer, mode);
    }

    /**
     * Returns whether or not the xfer mode can support treating coverage as alpha
     */
    virtual bool supportsCoverageAsAlpha() const;

    /**
     *  The same as calling xfermode->supportsCoverageAsAlpha(), except that this also checks if
     *  the xfermode is NULL, and if so, treats it as kSrcOver_Mode.
     */
    static bool SupportsCoverageAsAlpha(const SkXfermode* xfer);

    enum SrcColorOpacity {
        // The src color is known to be opaque (alpha == 255)
        kOpaque_SrcColorOpacity = 0,
        // The src color is known to be fully transparent (color == 0)
        kTransparentBlack_SrcColorOpacity = 1,
        // The src alpha is known to be fully transparent (alpha == 0)
        kTransparentAlpha_SrcColorOpacity = 2,
        // The src color opacity is unknown
        kUnknown_SrcColorOpacity = 3
    };

    /**
     * Returns whether or not the result of the draw with the xfer mode will be opaque or not. The
     * input to this call is an enum describing known information about the opacity of the src color
     * that will be given to the xfer mode.
     */
    virtual bool isOpaque(SrcColorOpacity opacityType) const;

    /**
     *  The same as calling xfermode->isOpaque(...), except that this also checks if
     *  the xfermode is NULL, and if so, treats it as kSrcOver_Mode.
     */
    static bool IsOpaque(const SkXfermode* xfer, SrcColorOpacity opacityType);

#if SK_SUPPORT_GPU
    /** Used by the SkXfermodeImageFilter to blend two colors via a GrFragmentProcessor.
        The input to the returned FP is the src color. The dst color is
        provided by the dst param which becomes a child FP of the returned FP. 
        It is legal for the function to return a null output. This indicates that
        the output of the blend is simply the src color.
     */
    virtual const GrFragmentProcessor* getFragmentProcessorForImageFilter(
                                                            const GrFragmentProcessor* dst) const;

    /** A subclass must implement this factory function to work with the GPU backend. 
        The xfermode will return a factory for which the caller will get a ref. It is up 
        to the caller to install it. XferProcessors cannot use a background texture.
      */
    virtual GrXPFactory* asXPFactory() const;
#endif

    SK_TO_STRING_PUREVIRT()
    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
    SK_DEFINE_FLATTENABLE_TYPE(SkXfermode)

    enum PM4fFlags {
        kSrcIsOpaque_PM4fFlag  = 1 << 0,
        kDstIsSRGB_PM4fFlag    = 1 << 1,
    };
    struct PM4fState {
        const SkXfermode* fXfer;
        uint32_t          fFlags;
    };
    typedef void (*PM4fProc1)(const PM4fState&, uint32_t dst[], const SkPM4f& src,
                              int count, const SkAlpha coverage[]);
    typedef void (*PM4fProcN)(const PM4fState&, uint32_t dst[], const SkPM4f src[],
                              int count, const SkAlpha coverage[]);

    static PM4fProc1 GetPM4fProc1(Mode, uint32_t flags);
    static PM4fProcN GetPM4fProcN(Mode, uint32_t flags);
    virtual PM4fProc1 getPM4fProc1(uint32_t flags) const;
    virtual PM4fProcN getPM4fProcN(uint32_t flags) const;

    enum U64Flags {
        kSrcIsOpaque_U64Flag  = 1 << 0,
        kDstIsFloat16_U64Flag = 1 << 1, // else U16 bit components
    };
    struct U64State {
        const SkXfermode* fXfer;
        uint32_t          fFlags;
    };
    typedef void (*U64Proc1)(const U64State&, uint64_t dst[], const SkPM4f& src, int count,
                             const SkAlpha coverage[]);
    typedef void (*U64ProcN)(const U64State&, uint64_t dst[], const SkPM4f src[], int count,
                             const SkAlpha coverage[]);
    static U64Proc1 GetU64Proc1(Mode, uint32_t flags);
    static U64ProcN GetU64ProcN(Mode, uint32_t flags);

    enum LCDFlags {
        kSrcIsOpaque_LCDFlag    = 1 << 0,   // else src(s) may have alpha < 1
        kSrcIsSingle_LCDFlag    = 1 << 1,   // else src[count]
        kDstIsLinearInt_LCDFlag = 1 << 2,   // else srgb/half-float
    };
    typedef void (*LCD32Proc)(uint32_t* dst, const SkPM4f* src, int count, const uint16_t lcd[]);
    typedef void (*LCD64Proc)(uint64_t* dst, const SkPM4f* src, int count, const uint16_t lcd[]);
    static LCD32Proc GetLCD32Proc(uint32_t flags);
    static LCD64Proc GetLCD64Proc(uint32_t) { return nullptr; }

protected:
    SkXfermode() {}
    /** The default implementation of xfer32/xfer16/xferA8 in turn call this
        method, 1 color at a time (upscaled to a SkPMColor). The default
        implementation of this method just returns dst. If performance is
        important, your subclass should override xfer32/xfer16/xferA8 directly.

        This method will not be called directly by the client, so it need not
        be implemented if your subclass has overridden xfer32/xfer16/xferA8
    */
    virtual SkPMColor xferColor(SkPMColor src, SkPMColor dst) const;

private:
    enum {
        kModeCount = kLastMode + 1
    };

    typedef SkFlattenable INHERITED;
};

#endif

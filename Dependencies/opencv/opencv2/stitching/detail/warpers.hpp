 /*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef __OPENCV_STITCHING_WARPERS_HPP__
#define __OPENCV_STITCHING_WARPERS_HPP__

#include "opencv2/core.hpp"
#include "opencv2/core/cuda.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv_modules.hpp"

namespace cv {
namespace detail {

class CV_EXPORTS RotationWarper
{
public:
    virtual ~RotationWarper() {}

    virtual Point2f warpPoint(const Point2f &pt, InputArray K, InputArray R) = 0;

    virtual Rect buildMaps(Size src_size, InputArray K, InputArray R, OutputArray xmap, OutputArray ymap) = 0;

    virtual Point warp(InputArray src, InputArray K, InputArray R, int interp_mode, int border_mode,
                       OutputArray dst) = 0;

    virtual void warpBackward(InputArray src, InputArray K, InputArray R, int interp_mode, int border_mode,
                              Size dst_size, OutputArray dst) = 0;

    virtual Rect warpRoi(Size src_size, InputArray K, InputArray R) = 0;

    virtual float getScale() const { return 1.f; }
    virtual void setScale(float) {}
};


struct CV_EXPORTS ProjectorBase
{
    void setCameraParams(InputArray K = Mat::eye(3, 3, CV_32F),
                         InputArray R = Mat::eye(3, 3, CV_32F),
                         InputArray T = Mat::zeros(3, 1, CV_32F));

    float scale;
    float k[9];
    float rinv[9];
    float r_kinv[9];
    float k_rinv[9];
    float t[3];
};


template <class P>
class CV_EXPORTS RotationWarperBase : public RotationWarper
{
public:
    Point2f warpPoint(const Point2f &pt, InputArray K, InputArray R);

    Rect buildMaps(Size src_size, InputArray K, InputArray R, OutputArray xmap, OutputArray ymap);

    Point warp(InputArray src, InputArray K, InputArray R, int interp_mode, int border_mode,
               OutputArray dst);

    void warpBackward(InputArray src, InputArray K, InputArray R, int interp_mode, int border_mode,
                      Size dst_size, OutputArray dst);

    Rect warpRoi(Size src_size, InputArray K, InputArray R);

    float getScale() const { return projector_.scale; }
    void setScale(float val) { projector_.scale = val; }

protected:

    // Detects ROI of the destination image. It's correct for any projection.
    virtual void detectResultRoi(Size src_size, Point &dst_tl, Point &dst_br);

    // Detects ROI of the destination image by walking over image border.
    // Correctness for any projection isn't guaranteed.
    void detectResultRoiByBorder(Size src_size, Point &dst_tl, Point &dst_br);

    P projector_;
};


struct CV_EXPORTS PlaneProjector : ProjectorBase
{
    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


class CV_EXPORTS PlaneWarper : public RotationWarperBase<PlaneProjector>
{
public:
    PlaneWarper(float scale = 1.f) { projector_.scale = scale; }

    Point2f warpPoint(const Point2f &pt, InputArray K, InputArray R, InputArray T);

    virtual Rect buildMaps(Size src_size, InputArray K, InputArray R, InputArray T, OutputArray xmap, OutputArray ymap);

    virtual Point warp(InputArray src, InputArray K, InputArray R, InputArray T, int interp_mode, int border_mode,
               OutputArray dst);

    Rect warpRoi(Size src_size, InputArray K, InputArray R, InputArray T);

protected:
    void detectResultRoi(Size src_size, Point &dst_tl, Point &dst_br);
};


struct CV_EXPORTS SphericalProjector : ProjectorBase
{
    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


// Projects image onto unit sphere with origin at (0, 0, 0).
// Poles are located at (0, -1, 0) and (0, 1, 0) points.
class CV_EXPORTS SphericalWarper : public RotationWarperBase<SphericalProjector>
{
public:
    SphericalWarper(float scale) { projector_.scale = scale; }

protected:
    void detectResultRoi(Size src_size, Point &dst_tl, Point &dst_br);
};


struct CV_EXPORTS CylindricalProjector : ProjectorBase
{
    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


// Projects image onto x * x + z * z = 1 cylinder
class CV_EXPORTS CylindricalWarper : public RotationWarperBase<CylindricalProjector>
{
public:
    CylindricalWarper(float scale) { projector_.scale = scale; }

protected:
    void detectResultRoi(Size src_size, Point &dst_tl, Point &dst_br)
    {
        RotationWarperBase<CylindricalProjector>::detectResultRoiByBorder(src_size, dst_tl, dst_br);
    }
};


struct CV_EXPORTS FisheyeProjector : ProjectorBase
{
    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


class CV_EXPORTS FisheyeWarper : public RotationWarperBase<FisheyeProjector>
{
public:
    FisheyeWarper(float scale) { projector_.scale = scale; }
};


struct CV_EXPORTS StereographicProjector : ProjectorBase
{
    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


class CV_EXPORTS StereographicWarper : public RotationWarperBase<StereographicProjector>
{
public:
    StereographicWarper(float scale) { projector_.scale = scale; }
};


struct CV_EXPORTS CompressedRectilinearProjector : ProjectorBase
{
    float a, b;

    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


class CV_EXPORTS CompressedRectilinearWarper : public RotationWarperBase<CompressedRectilinearProjector>
{
public:
    CompressedRectilinearWarper(float scale, float A = 1, float B = 1)
    {
        projector_.a = A;
        projector_.b = B;
        projector_.scale = scale;
    }
};


struct CV_EXPORTS CompressedRectilinearPortraitProjector : ProjectorBase
{
    float a, b;

    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


class CV_EXPORTS CompressedRectilinearPortraitWarper : public RotationWarperBase<CompressedRectilinearPortraitProjector>
{
public:
   CompressedRectilinearPortraitWarper(float scale, float A = 1, float B = 1)
   {
       projector_.a = A;
       projector_.b = B;
       projector_.scale = scale;
   }
};


struct CV_EXPORTS PaniniProjector : ProjectorBase
{
    float a, b;

    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


class CV_EXPORTS PaniniWarper : public RotationWarperBase<PaniniProjector>
{
public:
   PaniniWarper(float scale, float A = 1, float B = 1)
   {
       projector_.a = A;
       projector_.b = B;
       projector_.scale = scale;
   }
};


struct CV_EXPORTS PaniniPortraitProjector : ProjectorBase
{
    float a, b;

    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


class CV_EXPORTS PaniniPortraitWarper : public RotationWarperBase<PaniniPortraitProjector>
{
public:
   PaniniPortraitWarper(float scale, float A = 1, float B = 1)
   {
       projector_.a = A;
       projector_.b = B;
       projector_.scale = scale;
   }

};


struct CV_EXPORTS MercatorProjector : ProjectorBase
{
    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


class CV_EXPORTS MercatorWarper : public RotationWarperBase<MercatorProjector>
{
public:
    MercatorWarper(float scale) { projector_.scale = scale; }
};


struct CV_EXPORTS TransverseMercatorProjector : ProjectorBase
{
    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


class CV_EXPORTS TransverseMercatorWarper : public RotationWarperBase<TransverseMercatorProjector>
{
public:
    TransverseMercatorWarper(float scale) { projector_.scale = scale; }
};


#ifdef HAVE_OPENCV_CUDAWARPING
class CV_EXPORTS PlaneWarperGpu : public PlaneWarper
{
public:
    PlaneWarperGpu(float scale = 1.f) : PlaneWarper(scale) {}

    Rect buildMaps(Size src_size, InputArray K, InputArray R, OutputArray xmap, OutputArray ymap)
    {
        Rect result = buildMaps(src_size, K, R, d_xmap_, d_ymap_);
        d_xmap_.download(xmap);
        d_ymap_.download(ymap);
        return result;
    }

    Rect buildMaps(Size src_size, InputArray K, InputArray R, InputArray T, OutputArray xmap, OutputArray ymap)
    {
        Rect result = buildMaps(src_size, K, R, T, d_xmap_, d_ymap_);
        d_xmap_.download(xmap);
        d_ymap_.download(ymap);
        return result;
    }

    Point warp(InputArray src, InputArray K, InputArray R, int interp_mode, int border_mode,
               OutputArray dst)
    {
        d_src_.upload(src);
        Point result = warp(d_src_, K, R, interp_mode, border_mode, d_dst_);
        d_dst_.download(dst);
        return result;
    }

    Point warp(InputArray src, InputArray K, InputArray R, InputArray T, int interp_mode, int border_mode,
               OutputArray dst)
    {
        d_src_.upload(src);
        Point result = warp(d_src_, K, R, T, interp_mode, border_mode, d_dst_);
        d_dst_.download(dst);
        return result;
    }

    Rect buildMaps(Size src_size, InputArray K, InputArray R, cuda::GpuMat & xmap, cuda::GpuMat & ymap);

    Rect buildMaps(Size src_size, InputArray K, InputArray R, InputArray T, cuda::GpuMat & xmap, cuda::GpuMat & ymap);

    Point warp(const cuda::GpuMat & src, InputArray K, InputArray R, int interp_mode, int border_mode,
               cuda::GpuMat & dst);

    Point warp(const cuda::GpuMat & src, InputArray K, InputArray R, InputArray T, int interp_mode, int border_mode,
               cuda::GpuMat & dst);

private:
    cuda::GpuMat d_xmap_, d_ymap_, d_src_, d_dst_;
};


class CV_EXPORTS SphericalWarperGpu : public SphericalWarper
{
public:
    SphericalWarperGpu(float scale) : SphericalWarper(scale) {}

    Rect buildMaps(Size src_size, InputArray K, InputArray R, OutputArray xmap, OutputArray ymap)
    {
        Rect result = buildMaps(src_size, K, R, d_xmap_, d_ymap_);
        d_xmap_.download(xmap);
        d_ymap_.download(ymap);
        return result;
    }

    Point warp(InputArray src, InputArray K, InputArray R, int interp_mode, int border_mode,
               OutputArray dst)
    {
        d_src_.upload(src);
        Point result = warp(d_src_, K, R, interp_mode, border_mode, d_dst_);
        d_dst_.download(dst);
        return result;
    }

    Rect buildMaps(Size src_size, InputArray K, InputArray R, cuda::GpuMat & xmap, cuda::GpuMat & ymap);

    Point warp(const cuda::GpuMat & src, InputArray K, InputArray R, int interp_mode, int border_mode,
               cuda::GpuMat & dst);

private:
    cuda::GpuMat d_xmap_, d_ymap_, d_src_, d_dst_;
};


class CV_EXPORTS CylindricalWarperGpu : public CylindricalWarper
{
public:
    CylindricalWarperGpu(float scale) : CylindricalWarper(scale) {}

    Rect buildMaps(Size src_size, InputArray K, InputArray R, OutputArray xmap, OutputArray ymap)
    {
        Rect result = buildMaps(src_size, K, R, d_xmap_, d_ymap_);
        d_xmap_.download(xmap);
        d_ymap_.download(ymap);
        return result;
    }

    Point warp(InputArray src, InputArray K, InputArray R, int interp_mode, int border_mode,
               OutputArray dst)
    {
        d_src_.upload(src);
        Point result = warp(d_src_, K, R, interp_mode, border_mode, d_dst_);
        d_dst_.download(dst);
        return result;
    }

    Rect buildMaps(Size src_size, InputArray K, InputArray R, cuda::GpuMat & xmap, cuda::GpuMat & ymap);

    Point warp(const cuda::GpuMat & src, InputArray K, InputArray R, int interp_mode, int border_mode,
               cuda::GpuMat & dst);

private:
    cuda::GpuMat d_xmap_, d_ymap_, d_src_, d_dst_;
};
#endif


struct SphericalPortraitProjector : ProjectorBase
{
    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


// Projects image onto unit sphere with origin at (0, 0, 0).
// Poles are located NOT at (0, -1, 0) and (0, 1, 0) points, BUT at (1, 0, 0) and (-1, 0, 0) points.
class CV_EXPORTS SphericalPortraitWarper : public RotationWarperBase<SphericalPortraitProjector>
{
public:
    SphericalPortraitWarper(float scale) { projector_.scale = scale; }

protected:
    void detectResultRoi(Size src_size, Point &dst_tl, Point &dst_br);
};

struct CylindricalPortraitProjector : ProjectorBase
{
    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


class CV_EXPORTS CylindricalPortraitWarper : public RotationWarperBase<CylindricalPortraitProjector>
{
public:
    CylindricalPortraitWarper(float scale) { projector_.scale = scale; }

protected:
    void detectResultRoi(Size src_size, Point &dst_tl, Point &dst_br)
    {
        RotationWarperBase<CylindricalPortraitProjector>::detectResultRoiByBorder(src_size, dst_tl, dst_br);
    }
};

struct PlanePortraitProjector : ProjectorBase
{
    void mapForward(float x, float y, float &u, float &v);
    void mapBackward(float u, float v, float &x, float &y);
};


class CV_EXPORTS PlanePortraitWarper : public RotationWarperBase<PlanePortraitProjector>
{
public:
    PlanePortraitWarper(float scale) { projector_.scale = scale; }

protected:
    void detectResultRoi(Size src_size, Point &dst_tl, Point &dst_br)
    {
        RotationWarperBase<PlanePortraitProjector>::detectResultRoiByBorder(src_size, dst_tl, dst_br);
    }
};

/////////////////////////////////////// OpenCL Accelerated Warpers /////////////////////////////////////

class CV_EXPORTS PlaneWarperOcl : public PlaneWarper
{
public:
    PlaneWarperOcl(float scale = 1.f) : PlaneWarper(scale) { }

    virtual Rect buildMaps(Size src_size, InputArray K, InputArray R, OutputArray xmap, OutputArray ymap)
    {
        return buildMaps(src_size, K, R, Mat::zeros(3, 1, CV_32FC1), xmap, ymap);
    }

    virtual Point warp(InputArray src, InputArray K, InputArray R, int interp_mode, int border_mode, OutputArray dst)
    {
        return warp(src, K, R, Mat::zeros(3, 1, CV_32FC1), interp_mode, border_mode, dst);
    }

    virtual Rect buildMaps(Size src_size, InputArray K, InputArray R, InputArray T, OutputArray xmap, OutputArray ymap);
    virtual Point warp(InputArray src, InputArray K, InputArray R, InputArray T, int interp_mode, int border_mode, OutputArray dst);
};

class CV_EXPORTS SphericalWarperOcl :  public SphericalWarper
{
public:
    SphericalWarperOcl(float scale) : SphericalWarper(scale) { }

    virtual Rect buildMaps(Size src_size, InputArray K, InputArray R, OutputArray xmap, OutputArray ymap);
    virtual Point warp(InputArray src, InputArray K, InputArray R, int interp_mode, int border_mode, OutputArray dst);
};

class CV_EXPORTS CylindricalWarperOcl :  public CylindricalWarper
{
public:
    CylindricalWarperOcl(float scale) : CylindricalWarper(scale) { }

    virtual Rect buildMaps(Size src_size, InputArray K, InputArray R, OutputArray xmap, OutputArray ymap);
    virtual Point warp(InputArray src, InputArray K, InputArray R, int interp_mode, int border_mode, OutputArray dst);
};

} // namespace detail
} // namespace cv

#include "warpers_inl.hpp"

#endif // __OPENCV_STITCHING_WARPERS_HPP__

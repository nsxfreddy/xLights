#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "Model.h"
#include "../Image.h"

class ImageModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        ImageModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~ImageModel();

        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                                   int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;

        virtual void DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, DrawGLUtils::xlAccumulator &tva, float& minx, float& miny, float& maxx, float& maxy, bool is_3d = false, const xlColor *color = NULL, bool allowSelected = true, bool highlightFirst = false) override;
        virtual void DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, DrawGLUtils::xl3Accumulator &tva, DrawGLUtils::xl3Accumulator& lva, bool is_3d = false, const xlColor *color = NULL, bool allowSelected = true, bool wiring = false, bool highlightFirst = false, int highlightpixel = 0) override;
        virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual void DisableUnusedProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }
        virtual int GetNumPhysicalStrings() const override { return 1; }
        virtual std::list<std::string> GetFileReferences() override;
        virtual bool CleanupFileLocations(xLightsFrame* frame) override;
        virtual std::list<std::string> CheckModelSettings() override;

    protected:
        //void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, int width, int height, bool active);
        void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, DrawGLUtils::xlAccumulator &tva, const xlColor *c,
                               float &x1, float &y1, float&x2, float&y2, float& x3, float& y3, float& x4, float& y4, bool active);
        void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, DrawGLUtils::xl3Accumulator &tva, const xlColor *c,
                               float &x1, float &y1, float &z1,
                               float &x2, float &y2, float &z2,
                               float &x3, float &y3, float &z3,
                               float &x4, float &y4, float &z4, bool active);
        int GetChannelValue(int channel);

        ImageModel(const ModelManager &manager);
        virtual void InitModel() override;

        bool _whiteAsAlpha;
        std::string _imageFile;
        std::map<std::string, Image*> _images;
        int _offBrightness;
};


#ifndef ARCHESMODEL_H
#define ARCHESMODEL_H

#include "Model.h"

class ArchesModel : public ModelWithScreenLocation<ThreePointScreenLocation>
{
    public:
        ArchesModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~ArchesModel();
    
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                                   int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
        virtual int GetNumPhysicalStrings() const override { return 1; }

    
        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual bool SupportsXlightsModel() override { return true; }
        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

    protected:
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetNumStrands() const override;
        virtual int CalcCannelsPerString() override;

    private:
        void SetArchCoord();
        int arc;
        int _gap = 0;
};

#endif // ARCHESMODEL_H

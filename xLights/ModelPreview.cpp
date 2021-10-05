/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/artprov.h>
#include <wx/config.h>

#ifdef __WXMAC__
    #include "OpenGL/gl.h"
#else
    #include <GL/gl.h>
#endif

#include "ModelPreview.h"
#include "models/Model.h"
#include "models/ViewObject.h"
#include "PreviewPane.h"
#include "DrawGLUtils.h"
#include "ColorManager.h"
#include "LayoutGroup.h"
#include "xLightsMain.h"
#include "models/ModelGroup.h"
#include "ExternalHooks.h"

#include <log4cpp/Category.hh>

BEGIN_EVENT_TABLE(ModelPreview, xlGLCanvas)
	EVT_MOTION(ModelPreview::mouseMoved)
	EVT_LEFT_DOWN(ModelPreview::mouseLeftDown)
	EVT_LEFT_UP(ModelPreview::mouseLeftUp)
	EVT_LEAVE_WINDOW(ModelPreview::mouseLeftWindow)
	EVT_RIGHT_DOWN(ModelPreview::rightClick)
	//EVT_KEY_DOWN(ModelPreview::keyPressed)
	//EVT_KEY_UP(ModelPreview::keyReleased)
	EVT_MOUSEWHEEL(ModelPreview::mouseWheelMoved)
	EVT_MIDDLE_DOWN(ModelPreview::mouseMiddleDown)
	EVT_MIDDLE_UP(ModelPreview::mouseMiddleUp)
	EVT_PAINT(ModelPreview::Paint)
    EVT_SYS_COLOUR_CHANGED(ModelPreview::OnSysColourChanged)
    EVT_LEFT_DCLICK(ModelPreview::mouseLeftDClick)
END_EVENT_TABLE()

const long ModelPreview::ID_VIEWPOINT2D = wxNewId();
const long ModelPreview::ID_VIEWPOINT3D = wxNewId();
const long ModelPreview::ID_PREVIEW_VIEWPOINT_DEFAULT_RESTORE = wxNewId();

static glm::mat4 Identity(glm::mat4(1.0f));

void ModelPreview::setupCameras()
{
    camera3d = new PreviewCamera(true);
    camera2d = new PreviewCamera(false);
}

void ModelPreview::SetCamera2D(int i)
{
    *camera2d = *(xlights->viewpoint_mgr.GetCamera2D(i));
}

void ModelPreview::SetCamera3D(int i)
{
    *camera3d = *(xlights->viewpoint_mgr.GetCamera3D(i));
    SetCameraPos(0,0,false,true);
    SetCameraView(0,0,false,true);
}

void ModelPreview::SaveDefaultCameraPosition()
{
    if (is_3d) {
        xlights->viewpoint_mgr.SetDefaultCamera3D(camera3d);
    }
    else {
        xlights->viewpoint_mgr.SetDefaultCamera2D(camera2d);
    }
    xlights->MarkEffectsFileDirty();
}

void ModelPreview::RestoreDefaultCameraPosition()
{
    if (is_3d)
    {
        auto camera = xlights->viewpoint_mgr.GetDefaultCamera3D();
        if (camera == nullptr) {
            Reset();
        }
        else
        {
            *camera3d = *camera;
            SetCameraPos(0, 0, false, true);
            SetCameraView(0, 0, false, true);
        }
    }
    else {
        auto camera = xlights->viewpoint_mgr.GetDefaultCamera2D();
        if (camera == nullptr) {
            Reset();
        }
        else {
            *camera2d = *camera;
        }
    }
    xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ModelPreview::RestoreDefaultCameraPosition");
}

void ModelPreview::SaveCurrentCameraPosition()
{
    wxTextEntryDialog dlg(this, "Enter a name for this ViewPoint", "ViewPoint Name", "");
    bool name_ok = false;
    while (!name_ok) {
        if (dlg.ShowModal() == wxID_OK) {
            const std::string name = dlg.GetValue().ToStdString();
            if (name != "" && xlights->viewpoint_mgr.IsNameUnique(name, is_3d)) {
                PreviewCamera* current_camera = (is_3d ? camera3d : camera2d);
                xlights->viewpoint_mgr.AddCamera( name, current_camera, is_3d );
                name_ok = true;
            }
            else {
                wxMessageDialog msgDlg(this, "Error: ViewPoint name is a duplicate or blank.", "Error", wxOK | wxCENTRE);
                msgDlg.ShowModal();
            }
        }
        else {
            name_ok = true; // cancelled operation
        }
    }
}

void ModelPreview::OnZoomGesture(wxZoomGestureEvent& event) {
    if (!event.IsGestureStart()) {
        float delta = (m_last_mouse_x - (event.GetZoomFactor() * 1000)) / 1000.0;
        SetZoomDelta(delta);
        if (xlights != nullptr) {
            if (xlights->GetPlayStatus() == PLAY_TYPE_STOPPED || xlights->GetPlayStatus() == PLAY_TYPE_MODEL_PAUSED || xlights->GetPlayStatus() == PLAY_TYPE_EFFECT_PAUSED) {
                Refresh();
                Update();
                if (xlights->GetPlayStatus() == PLAY_TYPE_MODEL_PAUSED || xlights->GetPlayStatus() == PLAY_TYPE_EFFECT_PAUSED) {
                    Render(&xlights->_seqData[xlights->GetCurrentPlayTime() / xlights->_seqData.FrameTime()][0]);
                }
            }
        }
    }
    m_last_mouse_x = (event.GetZoomFactor() * 1000);
}

void ModelPreview::mouseMoved(wxMouseEvent& event) {
	if (m_mouse_down) {
		int delta_x = event.GetPosition().x - m_last_mouse_x;
		int delta_y = event.GetPosition().y - m_last_mouse_y;
		SetCameraView(delta_x, delta_y, false);
        if (xlights != nullptr) {
            if (xlights->GetPlayStatus() == PLAY_TYPE_STOPPED || xlights->GetPlayStatus() == PLAY_TYPE_MODEL_PAUSED || xlights->GetPlayStatus() == PLAY_TYPE_EFFECT_PAUSED || currentModel != "&---none---&") {
                Refresh();
                Update();
                if (xlights->GetPlayStatus() == PLAY_TYPE_MODEL_PAUSED || xlights->GetPlayStatus() == PLAY_TYPE_EFFECT_PAUSED) {
                    Render(&xlights->_seqData[xlights->GetCurrentPlayTime() / xlights->_seqData.FrameTime()][0]);
                }
            }
        }
    } else if (m_wheel_down) {
        float new_x = event.GetX() - m_last_mouse_x;
        float new_y = event.GetY() - m_last_mouse_y;
        if (!is_3d) {
            new_y *= -1.0f;
        }
        if (!is_3d) {
            SetPan(new_x / GetZoom(), new_y / GetZoom(), 0.0f);
        }
        else {
            // account for grid rotation
            float angleX = glm::radians(GetCameraRotationX());
            float angleY = glm::radians(GetCameraRotationY());
            float delta_x = 0.0f;
            float delta_y = 0.0f;
            float delta_z = 0.0f;
            bool top_view = (angleX > glm::radians(45.0f)) && (angleX < glm::radians(135.0f));
            bool bottom_view = (angleX > glm::radians(225.0f)) && (angleX < glm::radians(315.0f));
            bool upside_down_view = (angleX >= glm::radians(135.0f)) && (angleX <= glm::radians(225.0f));
            if (top_view) {
                delta_x = new_x * std::cos(angleY) - new_y * std::sin(angleY);
                delta_z = new_y * std::cos(angleY) + new_x * std::sin(angleY);
            }
            else if (bottom_view) {
                delta_x = -new_x * std::sin(angleY) - new_y * std::cos(angleY);
                delta_z = new_y * std::sin(angleY) - new_x * std::cos(angleY);
            }
            else {
                delta_x = new_x * std::cos(angleY);
                delta_y = new_y;
                delta_z = new_x * std::sin(angleY);
                if (!upside_down_view && is_3d) {
                    delta_y *= -1.0f;
                }
            }
            delta_x *= GetZoom() * 2.0f;
            delta_y *= GetZoom() * 2.0f;
            delta_z *= GetZoom() * 2.0f;
            SetPan(delta_x, delta_y, delta_z);
        }
        m_last_mouse_x = event.GetX();
        m_last_mouse_y = event.GetY();
        if (xlights != nullptr) {
            if (xlights->GetPlayStatus() == PLAY_TYPE_STOPPED || xlights->GetPlayStatus() == PLAY_TYPE_MODEL_PAUSED || xlights->GetPlayStatus() == PLAY_TYPE_EFFECT_PAUSED || currentModel != "&---none---&") {
                Refresh();
                Update();
                if (xlights->GetPlayStatus() == PLAY_TYPE_MODEL_PAUSED || xlights->GetPlayStatus() == PLAY_TYPE_EFFECT_PAUSED) {
                    Render(&xlights->_seqData[xlights->GetCurrentPlayTime() / xlights->_seqData.FrameTime()][0]);
                }
            }
        }
    }
    Model *model = xlights ? xlights->GetModel(currentModel) : nullptr;
    if (model != nullptr && !is_3d) {
        double x = event.GetPosition().x;
        double y = event.GetPosition().y;

        int w, h;
        GetSize(&w, &h);

        x -= camera2d->GetPanX() * camera2d->GetZoom();
        y += camera2d->GetPanY() * camera2d->GetZoom();

        x -= (double)w / 2.0;
        y -= (double)h / 2.0;

        x /= camera2d->GetZoom();
        y /= camera2d->GetZoom();

        x += (double)w / 2.0;
        y += (double)h / 2.0;

        wxString tip = model->GetNodeNear(this, wxPoint(x,y), true);
        SetToolTip(tip);
    }

    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

void ModelPreview::mouseLeftDown(wxMouseEvent& event) {
    SetFocus();
	m_mouse_down = true;
	m_last_mouse_x = event.GetX();
	m_last_mouse_y = event.GetY();

	event.ResumePropagation(1);
	event.Skip(); // continue the event
}

void ModelPreview::mouseLeftDClick(wxMouseEvent& event)
{
    wxTopLevelWindow* tlw = dynamic_cast<wxTopLevelWindow*>(GetParent()->GetParent());

    if (tlw==nullptr)
    {
        tlw = dynamic_cast<wxTopLevelWindow*>(GetParent());
    }

    if (tlw != nullptr)
    {
        if (tlw->IsMaximized())
        {
            tlw->Restore();
        }
        else {
            tlw->Maximize();
        }
    }
    else
    {

    }
}

void ModelPreview::mouseLeftUp(wxMouseEvent& event) {
	m_mouse_down = false;
	SetCameraView(0, 0, true);

	event.ResumePropagation(1);
	event.Skip(); // continue the event
}

void ModelPreview::mouseLeftWindow(wxMouseEvent& event) {
    m_mouse_down = false;
    m_wheel_down = false;
    event.ResumePropagation(1);
    event.Skip (); // continue the event
}

ModelGroup* ModelPreview::GetSelectedModelGroup()
{
    if (xlights != nullptr)
    {
        return xlights->GetSelectedModelGroup();
    }
    return nullptr;
}

const std::vector<Model*> &ModelPreview::GetModels() {
    tmpModelList.clear();
    if (xlights) {
        if (currentLayoutGroup == "Default") {
            if (additionalModel == nullptr) {
                //wxASSERT(ValidateModels(xlights->PreviewModels, xlights->AllModels));
                return xlights->PreviewModels;
            } else {
                tmpModelList = xlights->PreviewModels;
            }
        } else if (currentLayoutGroup == "All Models") {
            for (auto a : xlights->AllModels) {
                if (a.second->GetDisplayAs() != "ModelGroup") {
                    tmpModelList.push_back(a.second);
                }
            }
        } else if (currentLayoutGroup == "Unassigned") {
            for (auto a : xlights->AllModels) {
                if (a.second->GetLayoutGroup() == "Unassigned") {
                    if (a.second->GetDisplayAs() != "ModelGroup") {
                        tmpModelList.push_back(a.second);
                    }
                }
            }
        } else {
            bool foundGrp = false;
            for (auto grp : xlights->LayoutGroups) {
                if (currentLayoutGroup == grp->GetName()) {
                    foundGrp = true;
                    if (additionalModel == nullptr) {
                        //wxASSERT(ValidateModels(grp->GetModels(), xlights->AllModels));
                        return grp->GetModels();
                    } else {
                        tmpModelList = grp->GetModels();
                    }
                    break;
                }
            }
            if (!foundGrp) {
                tmpModelList = xlights->PreviewModels;
            }
        }

        // Only in debug builds but this really should all be valid or bad things will happen
        //wxASSERT(ValidateModels(tmpModelList, xlights->AllModels));
    }
    if (additionalModel != nullptr) {
        tmpModelList.push_back(additionalModel);
    }

    return tmpModelList;
}

bool ModelPreview::ValidateModels(const std::vector<Model*>models, const ModelManager& mm)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    for (const auto& it : models) {
        if (it->GetDisplayAs() != "SubModel") {
            bool found = false;
            for (auto it2 : mm) {
                if (it2.second == it) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // pointer to a non-existent model found ... not good
                wxASSERT(false);
                logger_base.error("Validating models in model preview %s found model that was not valid. This may crash!!!!", (const char*)GetName().c_str());
                return false;
            }
        }
    }
    //logger_base.debug("Validating models in model preview %s ALL OK", (const char*)GetName().c_str());
    return true;
}

// Checks that all models in the preview are valid
bool ModelPreview::ValidateModels(const ModelManager& mm)
{
    // this will actually double validate in debug builds
    return ValidateModels(GetModels(), mm);
}

void ModelPreview::SetModel(const Model* model, bool wiring, bool highlightFirst) {
    if (model) {
        _wiring = wiring;
        _highlightFirst = highlightFirst;
        this->xlights = model->GetModelManager().GetXLightsFrame();
        currentModel = model->GetName();
    } else {
        _wiring = false;
        _highlightFirst = false;
        currentModel = "";
    }
}

void ModelPreview::mouseWheelMoved(wxMouseEvent& event) {
    if (event.GetWheelRotation() == 0) {
        //rotation of 0 is sometimes generated for other gestures (pinch/zoom), ignore
        return;
    }
    if (!m_wheel_down) {
        bool fromTrackPad = IsMouseEventFromTouchpad();
        if (!fromTrackPad || event.ControlDown()) {
            float delta = event.GetWheelRotation() > 0 ? -0.1f : 0.1f;
            if (fromTrackPad) {
                float f = event.GetWheelRotation();
                delta = -f / 100.0f;
            }
            SetZoomDelta(delta);
        }
        else {
            float delta_x = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? 0 : -event.GetWheelRotation();
            float delta_y = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL ? -event.GetWheelRotation() : 0;
            if (is_3d) {
                if (event.ShiftDown()) {
                    SetPan(delta_x, delta_y, 0.0f);
                } else {
                    SetCameraView(delta_x, delta_y, false);
                    SetCameraView(0, 0, true);
                }
            } else {
                if (!fromTrackPad) {
                    delta_x *= GetZoom() * 2.0f;
                    delta_y *= GetZoom() * 2.0f;
                }
                SetPan(delta_x, delta_y, 0.0f);
                m_last_mouse_x = event.GetX();
                m_last_mouse_y = event.GetY();
            }
        }
    }
    if (xlights != nullptr) {
        if (xlights->GetPlayStatus() == PLAY_TYPE_STOPPED || xlights->GetPlayStatus() == PLAY_TYPE_MODEL_PAUSED || xlights->GetPlayStatus() == PLAY_TYPE_EFFECT_PAUSED) {
            Refresh();
            Update();
            if (xlights->GetPlayStatus() == PLAY_TYPE_MODEL_PAUSED || xlights->GetPlayStatus() == PLAY_TYPE_EFFECT_PAUSED) {
                Render(&xlights->_seqData[xlights->GetCurrentPlayTime() / xlights->_seqData.FrameTime()][0]);
            }
        }
    }
    event.ResumePropagation(1);
    event.Skip(); // continue the event
}

void ModelPreview::mouseMiddleDown(wxMouseEvent& event) {
    m_wheel_down = true;
    m_last_mouse_x = event.GetX();
    m_last_mouse_y = event.GetY();
}

void ModelPreview::mouseMiddleUp(wxMouseEvent& event) {
    m_wheel_down = false;
}

void ModelPreview::Paint(wxPaintEvent& event)
{
    DoPaint();
}
void ModelPreview::DoPaint()
{
    if (mIsDrawing) return;

    //if(!mIsInitialized) { InitializeGLCanvas(); }
    //SetCurrentGLContext();
    //wxPaintDC(this);

    if (currentModel == "&---none---&") {
        if (!StartDrawing(mPointSize, true)) return;
        Render();
        EndDrawing();
    } else {
        Model *model = xlights ? xlights->GetModel(currentModel) : nullptr;
        if (model != nullptr) {
            if (is_3d) {
                RenderModel(model, _wiring, _highlightFirst);
            } else {
                model->DisplayEffectOnWindow(this, 2);
            }
        } else {
            if (!StartDrawing(mPointSize, true)) return;
            EndDrawing();
        }
    }
}

void ModelPreview::RenderModels(const std::vector<Model*>& models, bool isModelSelected, bool highlightFirst)
{
    float minx = 999999;
    float maxx = -999999;
    float miny = 999999;
    float maxy = -999999;
    bool group = false;
    const xlColor* defColor = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_DEFAULT);
    const xlColor* selColor = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_SELECTED);
    const xlColor* overlapColor = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_OVERLAP);
    for (auto m : models) {
        if (xlights->AllModels.IsModelValid(m) || xlights->IsNewModel(m)) { // this IsModelValid should not be necessary but we are getting crashes due to invalid models

            if (xlights->IsNewModel(m)) {
                xlights->AddTraceMessage("ModelPreview::RenderModels IsNewModel was true.");
            }

            if (m->GroupSelected) {
                group = true;
            }

            const xlColor* color = defColor;
            if (m->Selected || m->GroupSelected) {
                color = selColor;
            }
            else if (m->Overlapping && isModelSelected) {
                color = overlapColor;
            }
            if (!allowSelected) {
                color = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_DEFAULT);
            }

            if (m->GetDisplayAs() == "SubModel" && !m->GroupSelected && !m->Selected)
            {
                // we dont display submodels if they are not selected
            }
            else
            {
                if (is_3d) {
                    m->DisplayModelOnWindow(this, solidAccumulator3d, transparentAccumulator3d, linesAccumulator3d, true, color, allowSelected, false, highlightFirst);
                }
                else {
                    float mix = 999999;
                    float max = -999999;
                    float miy = 999999;
                    float may = -999999;
                    m->DisplayModelOnWindow(this, solidAccumulator, transparentAccumulator, mix, miy, max, may, false, color, allowSelected, highlightFirst);
                    if (color == selColor)
                    {
                        if (mix < minx) minx = mix;
                        if (miy < miny) miny = miy;
                        if (max > maxx) maxx = max;
                        if (may > maxy) maxy = may;
                    }
                    // FIXME:  Delete when not needed for debugging
                    //if ((*PreviewModels)[i]->Highlighted) {
                    //    (*PreviewModels)[i]->GetModelScreenLocation().DrawBoundingBox(accumulator);
                    //}
                }

                // Because submodels are not always in the list (but sometimes are) I am going to have to go looing for selected submodels
                if (allowSelected) {
                    color = selColor;
                    for (auto& sm : m->GetSubModels())
                    {
                        if (sm->GroupSelected) {
                            group = true;
                        }

                        if (sm->GroupSelected || sm->Selected)
                        {
                            float mix = 999999;
                            float max = -999999;
                            float miy = 999999;
                            float may = -999999;
                            if (is_3d) {
                                sm->DisplayModelOnWindow(this, solidAccumulator3d, transparentAccumulator3d, linesAccumulator3d, true, color, allowSelected, false, highlightFirst);
                            }
                            else {
                                sm->DisplayModelOnWindow(this, solidAccumulator, transparentAccumulator, mix, miy, max, may, false, color, allowSelected, highlightFirst);
                            }
                            if (color == selColor)
                            {
                                if (mix < minx) minx = mix;
                                if (miy < miny) miny = miy;
                                if (max > maxx) maxx = max;
                                if (may > maxy) maxy = may;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            xlights->AddTraceMessage("ModelPreview::RenderModels found an invalid model ... skipped.");
            wxASSERT(false); // why did we get here
        }
    }

    auto mg = GetSelectedModelGroup();
    if (minx != 999999 && !Is3D() && mg != nullptr && xlights->AllModels.IsModelValid(mg))
    {
        int offx = 0;
        int offy = 0;
        offx = mg->GetXCentreOffset();
        offy = mg->GetYCentreOffset();

        DrawGroupCentre((minx + maxx) / 2.0 + (offx * (maxx - minx)) / 2000.0, (miny + maxy) / 2.0 + (offy * (maxy - miny)) / 2000.0);
    }
}

void ModelPreview::DrawGroupCentre(float x, float y)
{
    solidAccumulator.AddVertex(x - 20, y, xlREDTRANSLUCENT);
    solidAccumulator.AddVertex(x + 20, y, xlREDTRANSLUCENT);
    solidAccumulator.Finish(GL_LINES);
    solidAccumulator.AddVertex(x, y - 20, xlREDTRANSLUCENT);
    solidAccumulator.AddVertex(x, y + 20, xlREDTRANSLUCENT);
    solidAccumulator.Finish(GL_LINES);
}

void ModelPreview::RenderModel(Model* m, bool wiring, bool highlightFirst, int highlightpixel)
{
    float minx = 999999;
    float maxx = -999999;
    float miny = 999999;
    float maxy = -999999;

    const xlColor* defColor = ColorManager::instance()->GetColorPtr(ColorManager::COLOR_MODEL_DEFAULT);

    if (StartDrawing(mPointSize)) {
        if (is_3d) {
            int oldpixelSize = m->GetPixelSize();
            if (oldpixelSize < 5) m->SetPixelSize(5);
            m->DisplayModelOnWindow(this, solidAccumulator3d, transparentAccumulator3d, linesAccumulator3d, true, defColor, false, wiring, highlightFirst, highlightpixel);
            m->SetPixelSize(oldpixelSize);
        }
        else {
            m->DisplayModelOnWindow(this, solidAccumulator, transparentAccumulator, minx, miny, maxx, maxy, false, defColor, false, highlightFirst);
        }
        EndDrawing();
    }
}

void ModelPreview::Render()
{
    const std::vector<Model*>& models = GetModels();
    if (!models.empty()) {
        bool isModelSelected = false;
        for (auto& m : models) {
            if (xlights->AllModels.IsModelValid(m) || xlights->IsNewModel(m)) { // this IsModelValid should not be necessary but we are getting crashes due to invalid models
                if (m->Selected || m->GroupSelected) {
                    isModelSelected = true;
                    break;
                }
            }
            else
            {
                wxASSERT(false); // why did we get here
            }
        }
        RenderModels(models, isModelSelected, _showFirstPixel);
    }

    // draw all the view objects
    if (is_3d && xlights != nullptr) {
        for (const auto& it : xlights->AllObjects) {
            ViewObject* view_object = it.second;
            view_object->Draw(this, solidViewObjectAccumulator, transparentViewObjectAccumulator, allowSelected);
        }
    }
}

void ModelPreview::Render(const unsigned char *data, bool swapBuffers/*=true*/) {
    float minx = 999999;
    float maxx = -999999;
    float miny = 999999;
    float maxy = -999999;
    if (StartDrawing(mPointSize)) {
        const std::vector<Model*> &models = GetModels();
        for (auto m : models) {
            int NodeCnt = m->GetNodeCount();
            for (size_t n = 0; n < NodeCnt; ++n) {
                int start = m->NodeStartChannel(n);
                m->SetNodeChannelValues(n, &data[start]);
            }
            if (is_3d)
                m->DisplayModelOnWindow(this, solidAccumulator3d, transparentAccumulator3d, linesAccumulator3d, true);
            else
                m->DisplayModelOnWindow(this, solidAccumulator, transparentAccumulator, minx, miny, maxx, maxy, false);
        }
        // draw all the view objects
        if (is_3d) {
            for (const auto& it : xlights->AllObjects) {
                ViewObject *view_object = it.second;
                view_object->Draw(this, solidViewObjectAccumulator, transparentViewObjectAccumulator, allowSelected);
            }
        }
        EndDrawing(swapBuffers);
    }
}

void ModelPreview::rightClick(wxMouseEvent& event) {
    if (xlights != nullptr)
    {
        if (currentLayoutGroup != "")
        {
            wxMenu mnu;
            if (allowPreviewChange) {
                wxMenuItem* item = mnu.Append(0x1001, "3D", wxEmptyString, wxITEM_CHECK);
                item->Check(is_3d);
                mnu.AppendSeparator();
                mnu.Append(1, "House Preview");
                int index = 2;
                for (auto a : xlights->LayoutGroups) {
                    mnu.Append(index++, a->GetName());
                }
                // ViewPoint menus
                mnu.AppendSeparator();
            }
            mnu.Append(0x2001, "Reset");
            if (allowPreviewChange) {
                mnu.Append(ID_PREVIEW_VIEWPOINT_DEFAULT_RESTORE, _("Restore Default ViewPoint"));
                mnu.AppendSeparator();
                if (is_3d) {
                    if (xlights->viewpoint_mgr.GetNum3DCameras() > 0) {
                        wxMenu* mnuViewPoint = new wxMenu();
                        for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum3DCameras(); ++i) {
                            PreviewCamera* camera = xlights->viewpoint_mgr.GetCamera3D(i);
                            mnuViewPoint->Append(camera->GetMenuId(), camera->GetName());
                        }
                        mnuViewPoint->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)& ModelPreview::OnPopup, nullptr, this);
                        mnu.Append(ID_VIEWPOINT3D, "Load ViewPoint", mnuViewPoint, "");
                    }
                }
                else {
                    if (xlights->viewpoint_mgr.GetNum2DCameras() > 0) {
                        wxMenu* mnuViewPoint = new wxMenu();
                        for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum2DCameras(); ++i) {
                            PreviewCamera* camera = xlights->viewpoint_mgr.GetCamera2D(i);
                            mnuViewPoint->Append(camera->GetMenuId(), camera->GetName());
                        }
                        mnuViewPoint->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)& ModelPreview::OnPopup, nullptr, this);
                        mnu.Append(ID_VIEWPOINT2D, "Load ViewPoint", mnuViewPoint, "");
                    }
                }
            }
            mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)& ModelPreview::OnPopup, nullptr, this);
            PopupMenu(&mnu);
        }
    }
}

void ModelPreview::OnPopup(wxCommandEvent& event)
{
    int id = event.GetId() - 1;
    if (xlights) {
        if (id == 0) {
            currentLayoutGroup = "Default";
            SetBackgroundBrightness(xlights->GetDefaultPreviewBackgroundBrightness(), xlights->GetDefaultPreviewBackgroundAlpha());
            SetbackgroundImage(xlights->GetDefaultPreviewBackgroundImage());
        } else if (id > 0 && id <= xlights->LayoutGroups.size()) {
            currentLayoutGroup = xlights->LayoutGroups[id - 1]->GetName();
            SetBackgroundBrightness(xlights->LayoutGroups[id - 1]->GetBackgroundBrightness(), xlights->LayoutGroups[id - 1]->GetBackgroundAlpha());
            SetbackgroundImage(xlights->LayoutGroups[id - 1]->GetBackgroundImage());
        }
    } else {
        //if xlights isn't set, just return as there is nothing we can do (shouldn't ever happen)
        return;
    }

    if (id == ID_PREVIEW_VIEWPOINT_DEFAULT_RESTORE - 1) {
        RestoreDefaultCameraPosition();
    }
    else  if (id == 0x2000) {
        Reset();
    } else if (id == 0x1000) {
        is_3d = !is_3d;
    } else if (is_3d) {
        if (xlights->viewpoint_mgr.GetNum3DCameras() > 0) {
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum3DCameras(); ++i) {
                if (event.GetId() == xlights->viewpoint_mgr.GetCamera3D(i)->GetMenuId()) {
                    SetCamera3D(i);
                    break;
                }
            }
        }
    } else {
        if (xlights->viewpoint_mgr.GetNum2DCameras() > 0) {
            for (size_t i = 0; i < xlights->viewpoint_mgr.GetNum2DCameras(); ++i) {
                if (event.GetId() == xlights->viewpoint_mgr.GetCamera2D(i)->GetMenuId()) {
                    SetCamera2D(i);
                    break;
                }
            }
        }
    }
    Refresh();
    Update();
}

// reset cameras
void ModelPreview::Reset()
{
    // Reset
    if (is_3d)
    {
        camera3d->Reset();
        SetCameraPos(0, 0, false, true);
        SetCameraView(0, 0, false, true);
    }
    else
    {
        camera2d->Reset();
    }
}

ModelPreview::ModelPreview(wxPanel* parent, xLightsFrame* xlights_, bool a, int styles, bool apc, bool showFirstPixel)
    : xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, styles, a ? "Layout" : "Preview", false),
    virtualWidth(0), virtualHeight(0), _display2DBox(false), _center2D0(false),
    image(nullptr), sprite(nullptr), allowSelected(a), allowPreviewChange(apc), mPreviewPane(nullptr),
    xlights(xlights_), currentModel("&---none---&"),  currentLayoutGroup("Default"), additionalModel(nullptr), is_3d(false), m_mouse_down(false), m_wheel_down(false),
    m_last_mouse_x(-1), m_last_mouse_y(-1), camera3d(nullptr), renderOrder(0), camera2d(nullptr), _showFirstPixel(showFirstPixel)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    setupCameras();

    _cameraView_latched_x = camera3d->GetAngleX();
    _cameraView_latched_y = camera3d->GetAngleY();

    _cameraPos_latched_x = camera3d->GetPosX();
    _cameraPos_latched_y = camera3d->GetPosY();

    if (!allowSelected) {
        EnableTouchEvents(wxTOUCH_ZOOM_GESTURE);
        Connect(wxEVT_GESTURE_ZOOM, (wxObjectEventFunction)&ModelPreview::OnZoomGesture, nullptr, this);
    }

    wxConfigBase* config = wxConfigBase::Get();
    config->Read("OGLRenderOrder", &renderOrder, 0);
}

ModelPreview::ModelPreview(wxPanel* parent, xLightsFrame *xl)
    : xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "ModelPreview", false),
    virtualWidth(0), virtualHeight(0), _display2DBox(false), _center2D0(false),
    image(nullptr), sprite(nullptr), allowSelected(false), allowPreviewChange(false), mPreviewPane(nullptr),
    xlights(xl), currentModel(""), currentLayoutGroup("Default"), additionalModel(nullptr), is_3d(false), m_mouse_down(false), m_wheel_down(false),
    m_last_mouse_x(-1), m_last_mouse_y(-1), camera3d(nullptr), renderOrder(0), camera2d(nullptr)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    setupCameras();

    _cameraView_latched_x = camera3d->GetAngleX();
    _cameraView_latched_y = camera3d->GetAngleY();

    _cameraPos_latched_x = camera3d->GetPosX();
    _cameraPos_latched_y = camera3d->GetPosY();

    EnableTouchEvents(wxTOUCH_ZOOM_GESTURE);
    Connect(wxEVT_GESTURE_ZOOM, (wxObjectEventFunction)&ModelPreview::OnZoomGesture, nullptr, this);

    wxConfigBase* config = wxConfigBase::Get();
    config->Read("OGLRenderOrder", &renderOrder, 0);
}

ModelPreview::~ModelPreview()
{
    if (camera2d) {
        delete camera2d;
    }
    if (camera3d) {
        delete camera3d;
    }

    if (image) {
        if (cache) {
            cache->AddTextureToDelete(image->getID());
            image->setID(0);
        }
        delete image;
    }
    if (sprite) {
        delete sprite;
    }
}

void ModelPreview::SetCanvasSize(int width,int height)
{
    SetVirtualCanvasSize(width, height);
}

void ModelPreview::SetVirtualCanvasSize(int width, int height) {
    virtualWidth = width;
    virtualHeight = height;
}

void ModelPreview::InitializePreview(wxString img, int brightness, int alpha, bool center2d0)
{
    _center2D0 = center2d0;
    if (img != mBackgroundImage) {
        if (image) {
            if (cache) {
                cache->AddTextureToDelete(image->getID());
                image->setID(0);
            }
            delete image;
            image = nullptr;
        }
        if (sprite) {
            delete sprite;
            sprite = nullptr;
        }
        mBackgroundImage = img;
        mBackgroundImageExists = wxFileExists(mBackgroundImage) && wxIsReadable(mBackgroundImage) ? true : false;
    }
    mBackgroundBrightness = brightness;
    mBackgroundAlpha = alpha;
}

static inline wxColor GetBackgroundColor() {
    wxColor c = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
#ifdef __WXOSX__
    if (!c.IsSolid()) {
        c = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR);
    }
    if (!c.IsSolid()) {
        c.Set(204, 204, 204);
    }
#endif
    return c;
}
void ModelPreview::InitializeGLCanvas()
{
    mIsInitialized = true;
}
void ModelPreview::InitializeGLContext()
{
    SetCurrentGLContext();

    if (allowSelected) {
        wxColor c = GetBackgroundColor();
        LOG_GL_ERRORV(glClearColor(c.Red()/255.0f, c.Green()/255.0f, c.Blue()/255.0, 1.0f));
    } else {
        LOG_GL_ERRORV(glClearColor(0.0, 0.0, 0.0, 1.0f)); // Black Background
    }
    LOG_GL_ERRORV(glEnable(GL_BLEND));
    LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void ModelPreview::OnSysColourChanged(wxSysColourChangedEvent& event) {
    if (mIsInitialized) {
        SetCurrentGLContext();
        wxColor c = GetBackgroundColor();
        LOG_GL_ERRORV(glClearColor(c.Red()/255.0f, c.Green()/255.0f, c.Blue()/255.0, 1.0f));
    }
}

void ModelPreview::SetOrigin()
{
}

void ModelPreview::SetScaleBackgroundImage(bool b) {
    scaleImage = b;
    Refresh();
}

void ModelPreview::SetbackgroundImage(wxString img)
{
    if (img != mBackgroundImage) {
        ObtainAccessToURL(img.ToStdString());
        if (image) {
            if (cache) {
                cache->AddTextureToDelete(image->getID());
                image->setID(0);
            }
            delete image;
            image = nullptr;
        }
        if (sprite) {
            delete sprite;
            sprite = nullptr;
        }
        mBackgroundImage = img;
        mBackgroundImageExists = wxFileExists(mBackgroundImage) && wxIsReadable(mBackgroundImage) ? true : false;
    }
}

void ModelPreview::SetBackgroundBrightness(int brightness, int alpha)
{
    mBackgroundBrightness = brightness;
    mBackgroundAlpha = alpha;
    if(mBackgroundBrightness < 0 || mBackgroundBrightness > 100) {
        mBackgroundBrightness = 100;
    }
    if(mBackgroundAlpha < 0 || mBackgroundAlpha > 100) {
        mBackgroundAlpha = 100;
    }
}

void ModelPreview::SetPointSize(wxDouble pointSize)
{
    mPointSize = pointSize;
    LOG_GL_ERRORV(glPointSize( mPointSize ));
}

double ModelPreview::calcPixelSize(double i) {
    double d = translateToBacking(i * currentPixelScaleFactor);
    if (d < 1.0) {
        d = 1.0;
    }
    return d;
}

bool ModelPreview::GetActive() const
{
    return mPreviewPane->GetActive();
}

void ModelPreview::render(const wxSize& size/*wxSize(0,0)*/)
{
    wxSize origSize(0, 0);
    wxSize origVirtSize(virtualWidth, virtualHeight);
    if (size != wxSize(0, 0)) {
        origSize = wxSize(mWindowWidth, mWindowHeight);
        mWindowWidth = ((float)size.GetWidth() / GetContentScaleFactor());
        mWindowHeight = ((float)size.GetHeight() / GetContentScaleFactor());
        float mult = float(mWindowWidth) / origSize.GetWidth();
        virtualWidth = int(mult * origVirtSize.GetWidth());
        virtualHeight = int(mult * origVirtSize.GetHeight());
    }

    DoPaint();

    if (origSize != wxSize(0, 0)) {
        mWindowWidth = origSize.GetWidth();
        mWindowHeight = origSize.GetHeight();
        virtualWidth = origVirtSize.GetWidth();
        virtualHeight = origVirtSize.GetHeight();
    }
}

void ModelPreview::SetActive(bool show) {
    if (show) {
        mPreviewPane->Show();
    }
    else {
        mPreviewPane->Hide();
    }
}

void ModelPreview::SetCameraView(int camerax, int cameray, bool latch, bool reset)
{
	if( reset ) {
        _cameraView_last_offsetx = 0;
        _cameraView_last_offsety = 0;
        _cameraView_latched_x = camera3d->GetAngleX();
        _cameraView_latched_y = camera3d->GetAngleY();
	} else {
        if (latch) {
            camera3d->SetAngleX(_cameraView_latched_x + _cameraView_last_offsetx);
            camera3d->SetAngleY(_cameraView_latched_y + _cameraView_last_offsety);
            _cameraView_latched_x = camera3d->GetAngleX();
            _cameraView_latched_y = camera3d->GetAngleY();
            _cameraView_last_offsetx = 0;
            _cameraView_last_offsety = 0;
        }
        else {
            camera3d->SetAngleX(_cameraView_latched_x + cameray / 2);
            camera3d->SetAngleY(_cameraView_latched_y + camerax / 2);
            _cameraView_last_offsetx = cameray / 2;
            _cameraView_last_offsety = camerax / 2;
        }
	}
}

void ModelPreview::SetCameraPos(int camerax, int cameraz, bool latch, bool reset)
{
	if( reset ) {
        _cameraPos_last_offsetx = 0;
        _cameraPos_last_offsety = 0;
        _cameraPos_latched_x = camera3d->GetPosX();
        _cameraPos_latched_y = camera3d->GetPosY();
	} else {
        if (latch) {
            camera3d->SetPosX(_cameraPos_latched_x + _cameraPos_last_offsetx);
            camera3d->SetPosY(_cameraPos_latched_y + _cameraPos_last_offsety);
            _cameraPos_latched_x = camera3d->GetPosX();
            _cameraPos_latched_y = camera3d->GetPosY();
        }
        else {
            camera3d->SetPosX(_cameraPos_latched_x + camerax);
            camera3d->SetPosY(_cameraPos_latched_y + cameraz);
            _cameraPos_last_offsetx = camerax;
            _cameraPos_last_offsety = cameraz;
        }
	}
}

float ModelPreview::GetCameraZoomForHandles() const
{
    if (is_3d) {
        return camera3d->GetZoom();
    }
    return 1.0;
}

int ModelPreview::GetHandleScale() const
{
    return xlights->GetModelHandleScale();
}

void ModelPreview::SetZoomDelta(float delta)
{
    if (is_3d) {
        camera3d->SetZoom(camera3d->GetZoom() * (1.0f + delta));
    }
    else {
        camera2d->SetZoom(camera2d->GetZoom() * (1.0f - delta));
        camera2d->SetZoomCorrX(((mWindowWidth * camera2d->GetZoom()) - mWindowWidth) / 2.0f);
        camera2d->SetZoomCorrY(((mWindowHeight * camera2d->GetZoom()) - mWindowHeight) / 2.0f);
    }
}

void ModelPreview::SetPan(float deltax, float deltay, float deltaz)
{
    if (is_3d) {
        camera3d->SetPanX(camera3d->GetPanX() + deltax);
        camera3d->SetPanY(camera3d->GetPanY() + deltay);
        camera3d->SetPanZ(camera3d->GetPanZ() + deltaz);
    } else {
        camera2d->SetPanX(camera2d->GetPanX() + deltax);
        camera2d->SetPanY(camera2d->GetPanY() + deltay);
    }
}

bool ModelPreview::StartDrawing(wxDouble pointSize, bool fromPaint)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!fromPaint && !IsShownOnScreen()) return false;
    if (!mIsInitialized) { InitializeGLCanvas(); }
    mIsInitialized = true;
    mPointSize = pointSize;
    mIsDrawing = true;
    SetCurrentGLContext();

    /*****************************   2D   ********************************/
    if (!is_3d) {
        glDisable(GL_DEPTH_TEST);
        float scale2d = 1.0f;
        float scale_corrx = 0.0f;
        float scale_corry = 0.0f;
        if (virtualWidth != 0 && virtualHeight != 0) {
            float scale2dh = (float)mWindowHeight / (float)virtualHeight;
            float scale2dw = (float)mWindowWidth / (float)virtualWidth;
            // maintain aspect ratio.... FIXME: maybe add as an option
            // centers the direction that is smaller
            if (scale2dh < scale2dw) {
                scale2d = scale2dh;
                scale_corrx = ((scale2dw*(float)virtualWidth - (scale2d*(float)virtualWidth)) * camera2d->GetZoom()) / 2.0f;
            }
            else {
                scale2d = scale2dw;
                scale_corry = ((scale2dh*(float)virtualHeight - (scale2d*(float)virtualHeight)) * camera2d->GetZoom()) / 2.0f;
            }
        }
        glm::mat4 ViewScale = glm::scale(glm::mat4(1.0f), glm::vec3(camera2d->GetZoom() * scale2d, camera2d->GetZoom() * scale2d, 1.0f));
        glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(camera2d->GetPanX()*camera2d->GetZoom() - camera2d->GetZoomCorrX() + scale_corrx, camera2d->GetPanY()*camera2d->GetZoom() - camera2d->GetZoomCorrY() + scale_corry, 0.0f));
        ViewMatrix = ViewTranslate * ViewScale;
        if (_center2D0) {
            glm::mat4 cTranslate =  glm::translate(glm::mat4(1.0f), glm::vec3(((float)virtualWidth) / 2.0f, 0.0f, 0.0f));
            ViewMatrix = ViewTranslate * ViewScale * cTranslate;
        }
        ProjMatrix = glm::ortho(0.0f, (float)mWindowWidth, 0.0f, (float)mWindowHeight);  // this must match prepare2DViewport call
        ProjViewMatrix = ProjMatrix * ViewMatrix;

        prepare2DViewport(0, mWindowHeight, mWindowWidth, 0);
        LOG_GL_ERRORV(glClearColor(0, 0, 0, 0));   // background color
        LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        LOG_GL_ERRORV(glPointSize(translateToBacking(mPointSize)));
        DrawGLUtils::SetCamera(ViewMatrix);
        DrawGLUtils::PushMatrix();
        currentPixelScaleFactor = 1.0;
        if (!allowSelected && virtualWidth > 0 && virtualHeight > 0
            && (virtualWidth != mWindowWidth || virtualHeight != mWindowHeight)) {
            currentPixelScaleFactor = scale2d;
            LOG_GL_ERRORV(glPointSize(calcPixelSize(mPointSize)));
        }
        solidAccumulator.AddRect(0, 0, virtualWidth, virtualHeight, xlBLACK);
        solidAccumulator.Finish(GL_TRIANGLES);
    } else {
        /*****************************   3D   ********************************/
        glm::mat4 ViewTranslatePan = glm::translate(glm::mat4(1.0f), glm::vec3(camera3d->GetPosX() + camera3d->GetPanX(), camera3d->GetPosY() + camera3d->GetPanY(), camera3d->GetPosZ() + camera3d->GetPanZ()));
        glm::mat4 ViewTranslateDistance = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, camera3d->GetDistance() * camera3d->GetZoom()));
        glm::mat4 ViewRotateX = glm::rotate(glm::mat4(1.0f), glm::radians(camera3d->GetAngleX()), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ViewRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(camera3d->GetAngleY()), glm::vec3(0.0f, 1.0f, 0.0f));
        ViewMatrix = ViewTranslateDistance * ViewRotateX * ViewRotateY * ViewTranslatePan;
        ProjMatrix = glm::perspective(glm::radians(45.0f), (float)translateToBacking(mWindowWidth) / (float)translateToBacking(mWindowHeight), 1.0f, 20000.0f);  // this must match prepare3DViewport call
        ProjViewMatrix = ProjMatrix * ViewMatrix;

        // FIXME: commented out for debugging speed
        // FIXME: transparent background does not draw correctly when depth testing enabled
        // enables depth testing to draw things in proper order
        glEnable(GL_DEPTH_TEST);
        LOG_GL_ERRORV(glClearColor(0, 0, 0, 0));   // background color
        LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        glDepthFunc(GL_LESS);

        prepare3DViewport(0, mWindowHeight, mWindowWidth, 0);
        LOG_GL_ERRORV(glPointSize(translateToBacking(mPointSize)));
        DrawGLUtils::SetCamera(ViewMatrix);
        DrawGLUtils::PushMatrix();
        currentPixelScaleFactor = 1.0;
    }

    if (mBackgroundImageExists) {
        if (image == nullptr) {
            logger_base.debug("Loading background image file %s for preview %s.",
                              (const char *)mBackgroundImage.c_str(),
                              (const char *)GetName().c_str());
            image = new Image(mBackgroundImage);
            logger_base.debug("    Loaded.");
            sprite = new xLightsDrawable(image);
        }
        float scaleh = 1.0;
        float scalew = 1.0;
        if (!scaleImage) {
            float nscaleh = 1.0;
            if (virtualHeight != 0) nscaleh = float(image->height) / float(virtualHeight);
            if (nscaleh == 0) nscaleh = 1.0;
            float nscalew = 1.0;
            if (virtualWidth != 0) nscalew = float(image->width) / float(virtualWidth);
            if (nscalew == 0) nscalew = 1.0;
            if (nscalew < nscaleh) {
                scaleh = 1.0;
                scalew = nscalew / nscaleh;
            } else {
                scaleh = nscaleh / nscalew;
                scalew = 1.0;
            }
        }
        solidAccumulator.PreAllocTexture(6);
        float x = 0;
        if (_center2D0) {
            x = -virtualWidth;
            x /= 2.0f;
        }
        float tx1 = 0;
        float tx2 = image->tex_coord_x;
        if (image->textureHeight != 0) {
            solidAccumulator.AddTextureVertex(x, 0, tx1, -0.5 / (image->textureHeight));
            solidAccumulator.AddTextureVertex(x + virtualWidth * scalew, 0, tx2, -0.5 / (image->textureHeight));
            solidAccumulator.AddTextureVertex(x, virtualHeight * scaleh, tx1, image->tex_coord_y);

            solidAccumulator.AddTextureVertex(x, virtualHeight * scaleh, tx1, image->tex_coord_y);
            solidAccumulator.AddTextureVertex(x + virtualWidth * scalew, 0, tx2, -0.5 / (image->textureHeight));
            solidAccumulator.AddTextureVertex(x + virtualWidth * scalew, virtualHeight * scaleh, tx2, image->tex_coord_y);
        }

        float i = mBackgroundBrightness;
        float a = mBackgroundAlpha * 255.0f;
        a /= 100;
        solidAccumulator.FinishTextures(GL_TRIANGLES, image->getID(), (uint8_t)a, i);
    }

    // Draw a box around the default area in 2D
    if (!is_3d && allowSelected && _display2DBox) {
        if (_center2D0) {
            float x = -virtualWidth;
            x /= 2.0f;
            transparentAccumulator.AddLinesRect(x, 0, x + virtualWidth - 1, virtualHeight - 1, xlGREENTRANSLUCENT);
        } else {
            transparentAccumulator.AddLinesRect(0, 0, virtualWidth - 1, virtualHeight - 1, xlGREENTRANSLUCENT);
        }
        transparentAccumulator.Finish(GL_LINES);
    }

    return true;
}

void ModelPreview::EndDrawing(bool swapBuffers/*=true*/)
{
    if (is_3d) {
        switch (renderOrder) {
            // 0 or 1 is preferred depending if you want floods shining ONTO glass windows (0) or through (1)
            // 3 or 4 draws the pixels first so they may have black bands around them and strong moire, but
            //      seems to work around some video card drivers that cause extreme banding
            // 5 is the 2019.03 order, but floods will look awful (black circles)
            // 2 is the 2019.04 order, floods are OK, no transparent windows
            case 0:
                DrawGLUtils::Draw(solidViewObjectAccumulator);
                DrawGLUtils::Draw(linesAccumulator3d);
                DrawGLUtils::Draw(solidAccumulator3d);
                DrawGLUtils::Draw(transparentViewObjectAccumulator);
                DrawGLUtils::Draw(transparentAccumulator3d);
                break;
            case 1:
                DrawGLUtils::Draw(solidViewObjectAccumulator);
                DrawGLUtils::Draw(linesAccumulator3d);
                DrawGLUtils::Draw(solidAccumulator3d);
                DrawGLUtils::Draw(transparentAccumulator3d);
                DrawGLUtils::Draw(transparentViewObjectAccumulator);
                break;
            case 2:
                DrawGLUtils::Draw(solidViewObjectAccumulator);
                DrawGLUtils::Draw(transparentViewObjectAccumulator);
                DrawGLUtils::Draw(linesAccumulator3d);
                DrawGLUtils::Draw(solidAccumulator3d);
                DrawGLUtils::Draw(transparentAccumulator3d);
                break;
            case 3:
                DrawGLUtils::Draw(linesAccumulator3d);
                DrawGLUtils::Draw(solidAccumulator3d);
                DrawGLUtils::Draw(solidViewObjectAccumulator);
                DrawGLUtils::Draw(transparentViewObjectAccumulator);
                DrawGLUtils::Draw(transparentAccumulator3d);
                break;
            case 4:
                DrawGLUtils::Draw(linesAccumulator3d);
                DrawGLUtils::Draw(solidAccumulator3d);
                DrawGLUtils::Draw(solidViewObjectAccumulator);
                DrawGLUtils::Draw(transparentAccumulator3d);
                DrawGLUtils::Draw(transparentViewObjectAccumulator);
                break;
            case 5:
                DrawGLUtils::Draw(linesAccumulator3d);
                DrawGLUtils::Draw(solidAccumulator3d);
                DrawGLUtils::Draw(transparentAccumulator3d);
                DrawGLUtils::Draw(solidViewObjectAccumulator);
                DrawGLUtils::Draw(transparentViewObjectAccumulator);
                break;
        }
    } else {
        DrawGLUtils::Draw(solidAccumulator);
        DrawGLUtils::Draw(transparentAccumulator);
    }
    DrawGLUtils::PopMatrix();
    if (swapBuffers) {
        LOG_GL_ERRORV(SwapBuffers());
    }
    solidViewObjectAccumulator.Reset();
    transparentViewObjectAccumulator.Reset();
    linesAccumulator3d.Reset();
    solidAccumulator3d.Reset();
    transparentAccumulator3d.Reset();
    solidAccumulator.Reset();
    transparentAccumulator.Reset();
    mIsDrawing = false;
}

void ModelPreview::AddBoundingBoxToAccumulator(int x1, int y1, int x2, int y2) {
    solidAccumulator.AddDottedLinesRect(x1, y1, x2, y2,
                                       ColorManager::instance()->GetColor(ColorManager::COLOR_LAYOUT_DASHES));
    solidAccumulator.Finish(GL_LINES);
}

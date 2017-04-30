#include "ViewsModelsPanel.h"

//(*InternalHeaders(ViewsModelsPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "../include/model-16.xpm"
#include "../include/timing-16.xpm"
#include "../include/eye-16.xpm"
#include "../include/eye-16_gray.xpm"
#include <wx/xml/xml.h>
#include "sequencer/SequenceElements.h"
#include "xLightsMain.h"
#include "SequenceViewManager.h"
#include <wx/dnd.h>

// This event is fired when a model is dropped between lists
wxDEFINE_EVENT(EVT_VMDROP, wxCommandEvent);

class VMDropSource : public wxDropSource
{
    ViewsModelsPanel* _window;
    bool _nonModels;
    bool _models;

public:

    VMDropSource(ViewsModelsPanel* window, bool models, bool nonModels) : wxDropSource(window)
    {
        _window = window;
        _nonModels = nonModels;
        _models = models;
    }

    virtual bool GiveFeedback(wxDragResult effect) override
    {
        int sx;
        int sy;
        _window->GetScreenPosition(&sx, &sy);
        wxPoint point = wxGetMousePosition() - wxPoint(sx, sy);

        if (_models)
        {
            if (_window->ListCtrlModels->GetRect().Contains(point) ||
                _window->ListCtrlNonModels->GetRect().Contains(point))
            {
                _window->SetCursor(wxCursor(wxCURSOR_HAND));
            }
            else
            {
                _window->SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
            }
        }
        else if (_nonModels)
        {
            if (_window->ListCtrlModels->GetRect().Contains(point))
            {
                _window->SetCursor(wxCursor(wxCURSOR_HAND));
            }
            else
            {
                _window->SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
            }
        }
        else
        {
            return false;
        }

        return true;
    }
};

#define TIMING_IMAGE 2
#define MODEL_IMAGE 3

//(*IdInit(ViewsModelsPanel)
const long ViewsModelsPanel::ID_STATICTEXT3 = wxNewId();
const long ViewsModelsPanel::ID_LISTCTRL1 = wxNewId();
const long ViewsModelsPanel::ID_BUTTON3 = wxNewId();
const long ViewsModelsPanel::ID_BUTTON4 = wxNewId();
const long ViewsModelsPanel::ID_BUTTON5 = wxNewId();
const long ViewsModelsPanel::ID_BUTTON6 = wxNewId();
const long ViewsModelsPanel::ID_STATICTEXT1 = wxNewId();
const long ViewsModelsPanel::ID_LISTCTRL_VIEWS = wxNewId();
const long ViewsModelsPanel::ID_BUTTON1 = wxNewId();
const long ViewsModelsPanel::ID_BUTTON2 = wxNewId();
const long ViewsModelsPanel::ID_STATICTEXT2 = wxNewId();
const long ViewsModelsPanel::ID_LISTCTRL_MODELS = wxNewId();
const long ViewsModelsPanel::ID_SCROLLEDWINDOW1 = wxNewId();
const long ViewsModelsPanel::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ViewsModelsPanel,wxPanel)
	//(*EventTable(ViewsModelsPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_LISTITEM_CHECKED, ViewsModelsPanel::OnListCtrlItemCheck)
    EVT_COMMAND(wxID_ANY, EVT_VMDROP, ViewsModelsPanel::OnDrop)
END_EVENT_TABLE()

ViewsModelsPanel::ViewsModelsPanel(xLightsFrame *frame, wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size) : _xlFrame(frame)
{
	//(*Initialize(ViewsModelsPanel)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer9->AddGrowableRow(0);
	ScrolledWindowViewsModels = new wxScrolledWindow(Panel_Sizer, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	FlexGridSizer10 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer10->AddGrowableCol(2);
	FlexGridSizer10->AddGrowableRow(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(1);
	StaticText3 = new wxStaticText(ScrolledWindowViewsModels, ID_STATICTEXT3, _("Available:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	ListCtrlNonModels = new wxListCtrl(ScrolledWindowViewsModels, ID_LISTCTRL1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL1"));
	FlexGridSizer2->Add(ListCtrlNonModels, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer10->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_AddAll = new wxButton(ScrolledWindowViewsModels, ID_BUTTON3, _(">>"), wxDefaultPosition, wxSize(32,30), 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer5->Add(Button_AddAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_AddSelected = new wxButton(ScrolledWindowViewsModels, ID_BUTTON4, _(">"), wxDefaultPosition, wxSize(32,32), 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer5->Add(Button_AddSelected, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_RemoveSelected = new wxButton(ScrolledWindowViewsModels, ID_BUTTON5, _("<"), wxDefaultPosition, wxSize(32,32), 0, wxDefaultValidator, _T("ID_BUTTON5"));
	FlexGridSizer5->Add(Button_RemoveSelected, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_RemoveAll = new wxButton(ScrolledWindowViewsModels, ID_BUTTON6, _("<<"), wxDefaultPosition, wxSize(32,32), 0, wxDefaultValidator, _T("ID_BUTTON6"));
	FlexGridSizer5->Add(Button_RemoveAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(1);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	FlexGridSizer6->AddGrowableRow(0);
	FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	FlexGridSizer7->AddGrowableRow(1);
	StaticText1 = new wxStaticText(ScrolledWindowViewsModels, ID_STATICTEXT1, _("View:"), wxDefaultPosition, wxSize(44,16), 0, _T("ID_STATICTEXT1"));
	FlexGridSizer7->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	ListCtrlViews = new wxCheckedListCtrl(ScrolledWindowViewsModels, ID_LISTCTRL_VIEWS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL, wxDefaultValidator, _T("ID_LISTCTRL_VIEWS"));
	ListCtrlViews->SetMinSize(wxDLG_UNIT(ScrolledWindowViewsModels,wxSize(100,50)));
	FlexGridSizer7->Add(ListCtrlViews, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer6->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_AddView = new wxButton(ScrolledWindowViewsModels, ID_BUTTON1, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer8->Add(Button_AddView, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_DeleteView = new wxButton(ScrolledWindowViewsModels, ID_BUTTON2, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer8->Add(Button_DeleteView, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer6->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer3->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(1);
	StaticText2 = new wxStaticText(ScrolledWindowViewsModels, ID_STATICTEXT2, _("Added:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer4->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	ListCtrlModels = new wxCheckedListCtrl(ScrolledWindowViewsModels, ID_LISTCTRL_MODELS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL_MODELS"));
	ListCtrlModels->SetMinSize(wxDLG_UNIT(ScrolledWindowViewsModels,wxSize(75,85)));
	FlexGridSizer4->Add(ListCtrlModels, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer10->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	ScrolledWindowViewsModels->SetSizer(FlexGridSizer10);
	FlexGridSizer10->Fit(ScrolledWindowViewsModels);
	FlexGridSizer10->SetSizeHints(ScrolledWindowViewsModels);
	FlexGridSizer9->Add(ScrolledWindowViewsModels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Sizer->SetSizer(FlexGridSizer9);
	FlexGridSizer9->Fit(Panel_Sizer);
	FlexGridSizer9->SetSizeHints(Panel_Sizer);
	FlexGridSizer1->Add(Panel_Sizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTCTRL1,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlNonModelsBeginDrag);
	Connect(ID_LISTCTRL1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlNonModelsItemSelect);
	Connect(ID_LISTCTRL1,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlNonModelsKeyDown);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsModelsPanel::OnButton_AddAllClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsModelsPanel::OnButton_AddSelectedClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsModelsPanel::OnButton_RemoveSelectedClick);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsModelsPanel::OnButton_RemoveAllClick);
	Connect(ID_LISTCTRL_VIEWS,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlViewsItemSelect);
	Connect(ID_LISTCTRL_VIEWS,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlViewsKeyDown);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsModelsPanel::OnButton_AddViewClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ViewsModelsPanel::OnButton_DeleteViewClick);
	Connect(ID_LISTCTRL_MODELS,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&ViewsModelsPanel::OnListView_ViewItemsBeginDrag);
	Connect(ID_LISTCTRL_MODELS,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&ViewsModelsPanel::OnListView_ViewItemsItemSelect);
	Connect(ID_LISTCTRL_MODELS,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&ViewsModelsPanel::OnListView_ViewItemsKeyDown);
	Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&ViewsModelsPanel::OnLeftUp);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&ViewsModelsPanel::OnResize);
	//*)

    Connect(wxID_ANY, EVT_VMDROP, (wxObjectEventFunction)&ViewsModelsPanel::OnDrop);

    ListCtrlViews->SetImages((char**)eye_16, (char**)eye_16_gray);

    _numViews = 0;
    _numModels = 0;
    _numNonModels = 0;
    _sequenceElements = nullptr;
    _mainViewsChoice = nullptr;

    ListCtrlModels->SetImages((char**)eye_16, (char**)eye_16_gray);
    ListCtrlModels->AddImage((char**)timing_16);
    ListCtrlModels->AddImage((char**)model_16);

    _imageList = new wxImageList(16, 16, true);
    _imageList->Add(wxIcon((char**)timing_16));
    _imageList->Add(wxIcon((char**)model_16));

    MyTextDropTarget *mdt = new MyTextDropTarget(this, ListCtrlModels, "Model");
    ListCtrlModels->SetDropTarget(mdt);

    mdt = new MyTextDropTarget(this, ListCtrlNonModels, "NonModel");
    ListCtrlNonModels->SetDropTarget(mdt);

    ValidateWindow();
}

ViewsModelsPanel::~ViewsModelsPanel()
{
	//(*Destroy(ViewsModelsPanel)
	//*)

    for (int i = 0; i < ListCtrlNonModels->GetItemCount(); ++i)
    {
        Element* e = (Element*)ListCtrlNonModels->GetItemData(i);
        if (e != nullptr && e->GetType() == ELEMENT_TYPE_MODEL && e->GetSequenceElements() == nullptr)
        {
            delete e;
        }
    }

    delete _imageList;
}

void ViewsModelsPanel::PopulateModels()
{
    int topM = ListCtrlModels->GetTopItem();
    int topN = ListCtrlNonModels->GetTopItem();

    int itemSize = 0;
    if (ListCtrlModels->GetItemCount() > 0)
    {
        wxRect rect;
        ListCtrlModels->GetItemRect(0, rect);
        itemSize = rect.GetHeight();
    }
    else
    {
        // need to add something so i can measure it

        ListCtrlModels->AppendColumn("Test");
        ListCtrlModels->InsertItem(0, "Test");

        wxRect rect;
        ListCtrlModels->GetItemRect(0, rect);
        itemSize = rect.GetHeight();
    }
    wxASSERT(itemSize != 0);

    int visibileM = ListCtrlModels->GetRect().GetHeight() / itemSize - 1;
    int visibileN = ListCtrlNonModels->GetRect().GetHeight() / itemSize - 1;

    for (int i = 0; i < ListCtrlNonModels->GetItemCount(); ++i)
    {
        Element* e = (Element*)ListCtrlNonModels->GetItemData(i);
        if (e != nullptr && e->GetType() == ELEMENT_TYPE_MODEL && e->GetSequenceElements() == nullptr)
        {
            delete e;
        }
    }

    ListCtrlNonModels->ClearAll();
    ListCtrlNonModels->SetImageList(_imageList, wxIMAGE_LIST_SMALL);

    wxListItem nm0;
    nm0.SetId(0);
    nm0.SetImage(-1);
    nm0.SetText(_(""));
    nm0.SetWidth(22);
    ListCtrlNonModels->InsertColumn(0, nm0);

    wxListItem nm1;
    nm1.SetId(1);
    nm1.SetText(_("Timing/Model"));
    nm1.SetWidth(130);
    ListCtrlNonModels->InsertColumn(1, nm1);

    ListCtrlModels->ClearAll();

    wxListItem col0;
    col0.SetId(0);
    col0.SetText(_(""));
    col0.SetWidth(28);
    ListCtrlModels->InsertColumn(0, col0);

    wxListItem col1;
    col1.SetId(1);
    col1.SetText(_(""));
    col1.SetWidth(22);
    ListCtrlModels->InsertColumn(1, col1);

    wxListItem col2;
    col2.SetId(2);
    col2.SetText(_("Timing/Model"));
    col2.SetWidth(130);
    ListCtrlModels->InsertColumn(2, col2);

    _numModels = 0;
    _numNonModels = 0;
    int current_view = _sequenceElements->GetCurrentView();
    for (int i = 0; i < _sequenceElements->GetElementCount(); i++)
    {
        Element* elem = _sequenceElements->GetElement(i);
        if (elem->GetType() == ELEMENT_TYPE_TIMING)
        {
            TimingElement *te = dynamic_cast<TimingElement*>(elem);
            if (current_view == MASTER_VIEW || _sequenceElements->TimingIsPartOfView(te, current_view))
            {
                AddTimingToList(elem);
            }
            else
            {
                AddTimingToNotList(elem);
            }
        }
    }

    if (current_view > 0)
    {
        _sequenceViewManager->SetSelectedView(current_view);
        SequenceView * view = _sequenceViewManager->GetSelectedView();
        if (view != nullptr)
        {
            _sequenceElements->AddMissingModelsToSequence(view->GetModelsString());
            auto models = view->GetModels();
            for (auto it = models.begin(); it != models.end(); ++it)
            {
                Element* elem = _sequenceElements->GetElement(*it);
                AddModelToList(elem);
            }

            // add everything that isnt in the view
            for (int i = 0; i < _sequenceElements->GetElementCount(); i++)
            {
                Element* elem = _sequenceElements->GetElement(i);
                if (elem->GetType() == ELEMENT_TYPE_MODEL && std::find(models.begin(), models.end(), elem->GetName()) == models.end())
                {
                    AddModelToNotList(elem);
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < _sequenceElements->GetElementCount(); i++)
        {
            Element* elem = _sequenceElements->GetElement(i);
            if (elem->GetType() == ELEMENT_TYPE_MODEL)
            {
                AddModelToList(elem);
            }
        }

        for (wxXmlNode* e = _modelGroups->GetChildren(); e != nullptr; e = e->GetNext())
        {
            if (e->GetName() == "modelGroup")
            {
                wxString name = e->GetAttribute("name");
                if (!_sequenceElements->ElementExists(name.ToStdString(), 0))
                {
                    ModelElement *me = new ModelElement(name.ToStdString());
                    AddModelToNotList(me);
                }
            }
        }

        for (wxXmlNode* e = _models->GetChildren(); e != nullptr; e = e->GetNext())
        {
            if (e->GetName() == "model")
            {
                wxString name = e->GetAttribute("name");
                if (!_sequenceElements->ElementExists(name.ToStdString(), 0))
                {
                    ModelElement *me = new ModelElement(name.ToStdString());
                    AddModelToNotList(me);
                }
            }
        }
    }

    if (topM + visibileM - 1< ListCtrlModels->GetItemCount())
    {
        ListCtrlModels->EnsureVisible(topM + visibileM - 1);
    }
    ListCtrlModels->EnsureVisible(topM);

    if (topN + visibileN - 1< ListCtrlNonModels->GetItemCount())
    {
        ListCtrlNonModels->EnsureVisible(topN + visibileN - 1);
    }
    ListCtrlNonModels->EnsureVisible(topN);
}

bool ViewsModelsPanel::IsModelAGroup(const std::string& modelname) const
{
    for (auto it = _modelGroups->GetChildren(); it != nullptr; it = it->GetNext())
    {
        if (it->GetName() == "modelGroup" && it->GetAttribute("name") == modelname)
        {
            return true;
        }
    }
    return false;
}

bool ViewsModelsPanel::IsItemSelected(wxListCtrl* ctrl, int item)
{
    return ctrl->GetItemState(item, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED;
}

bool ViewsModelsPanel::DeselectItem(wxListCtrl* ctrl, int item)
{
    return ctrl->SetItemState(item, 0, wxLIST_STATE_SELECTED);
}

void ViewsModelsPanel::OnListView_ViewItemsBeginDrag(wxListEvent& event)
{
    if (ListCtrlModels->GetSelectedItemCount() == 0) return;

    _dragRowModel = true;
    _dragRowNonModel = false;

    wxString drag = "Model";
    for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListCtrlModels, i))
        {
            drag += "," + ListCtrlModels->GetItemText(i, 1);
        }
    }

    wxTextDataObject my_data(drag);
    VMDropSource dragSource(this, true, false);
    dragSource.SetData(my_data);
    dragSource.DoDragDrop(wxDrag_DefaultMove);
    SetCursor(wxCURSOR_ARROW);

    ValidateWindow();
}

void ViewsModelsPanel::OnListView_ViewItemsItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void ViewsModelsPanel::OnListView_ViewItemsKeyDown(wxListEvent& event)
{
    auto key = event.GetKeyCode();
    if (key == WXK_LEFT || key == WXK_DELETE || key == WXK_NUMPAD_DELETE)
    {
        RemoveSelectedModels();
    }

    ValidateWindow();
}

void ViewsModelsPanel::RemoveSelectedModels()
{
    if (_seqData->NumFrames() == 0) return;

    if (_sequenceElements->GetCurrentView() == MASTER_VIEW)
    {
        bool hasEffects = false;
        for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i)
        {
            if (IsItemSelected(ListCtrlModels, i))
            {
                Element* e = (Element*)ListCtrlModels->GetItemData(i);
                if (e->HasEffects())
                {
                    hasEffects = true;
                    break;
                }
            }
        }

        if (!hasEffects || wxMessageBox("Delete all effects and layers for the selected model(s)?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxYES)
        {
            for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i)
            {
                if (IsItemSelected(ListCtrlModels, i))
                {
                    // Got a selected item so handle it
                    Element* e = (Element*)ListCtrlModels->GetItemData(i);
                    _sequenceElements->DeleteElement(e->GetName());
                }
            }
        }
    }
    else
    {
        for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i)
        {
            if (IsItemSelected(ListCtrlModels, i))
            {
                // Got a selected item so handle it
                Element* e = (Element*)ListCtrlModels->GetItemData(i);
                if (e->GetType() == ELEMENT_TYPE_MODEL)
                {
                    _sequenceElements->DeleteElementFromView(e->GetName(), _sequenceElements->GetCurrentView());
                }
                else
                {
                    _sequenceElements->DeleteTimingFromView(e->GetName(), _sequenceElements->GetCurrentView());
                    _sequenceElements->SetTimingVisibility(_sequenceElements->GetViewName(_sequenceElements->GetCurrentView()));
                }
            }
        }
        UpdateModelsForSelectedView();
    }

    MarkViewsChanged();

    PopulateModels();

    // Update Grid
    _xlFrame->DoForceSequencerRefresh();
}

void ViewsModelsPanel::AddSelectedModels(int pos)
{
    if (_seqData->NumFrames() == 0) return;

    int p = pos;

    std::string type = "model";
    int current_view = _sequenceElements->GetCurrentView();

    if (p == -1)
    {
        p = _sequenceElements->GetElementCount();
    }
    else
    {
        p -= GetTimingCount();
        if (p < 0) p = 0;
    }

    if (current_view == MASTER_VIEW)
    {
        int selcnt = 0;

        for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i)
        {
            if (IsItemSelected(ListCtrlNonModels, i))
            {
                Element* ee = (Element*)ListCtrlNonModels->GetItemData(i);
                if (ee != nullptr && ee->GetType() != ELEMENT_TYPE_TIMING)
                {
                    Element* e = _sequenceElements->AddElement(p + selcnt, ListCtrlNonModels->GetItemText(i, 1).ToStdString(), type, true, false, false, false);
                    if (e != nullptr)
                    {
                        e->AddEffectLayer();
                    }
                    selcnt++;
                }
            }
        }
    }
    else
    {
        int selcnt = 0;
        auto view = _sequenceViewManager->GetSelectedView();
        for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i)
        {
            if (IsItemSelected(ListCtrlNonModels, i))
            {
                Element* ee = (Element*)ListCtrlNonModels->GetItemData(i);
                if (ee != nullptr && ee->GetType() != ELEMENT_TYPE_TIMING)
                {
                    view->AddModel(ListCtrlNonModels->GetItemText(i, 1).ToStdString(), p + selcnt);
                    selcnt++;
                }
            }
        }

        _sequenceElements->AddMissingModelsToSequence(view->GetModelsString());
        for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i)
        {
            if (IsItemSelected(ListCtrlNonModels, i))
            {
                Element* e = _sequenceElements->GetElement(ListCtrlNonModels->GetItemText(i, 1).ToStdString());
                if (e != nullptr && e->GetType() != ELEMENT_TYPE_TIMING)
                {
                    e->SetVisible(true);
                }
            }
        }

        std::string modelsString = _sequenceElements->GetViewModels(view->GetName());
        _sequenceElements->PopulateView(modelsString, current_view);

        std::vector<std::string> timings;
        for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i)
        {
            if (IsItemSelected(ListCtrlNonModels, i))
            {
                Element* ee = (Element*)ListCtrlNonModels->GetItemData(i);
                if (ee != nullptr && ee->GetType() == ELEMENT_TYPE_TIMING)
                {
                    timings.push_back(ListCtrlNonModels->GetItemText(i, 1).ToStdString());
                }
            }
        }

        if (timings.size() > 0)
        {
            _sequenceElements->AddViewToTimings(timings, view->GetName());
        }

        _sequenceElements->SetTimingVisibility(view->GetName());
    }
    MarkViewsChanged();

    PopulateModels();

    // Update Grid
    _xlFrame->DoForceSequencerRefresh();
}

void ViewsModelsPanel::OnButton_AddAllClick(wxCommandEvent& event)
{
    for (int i = 0; i < ListCtrlNonModels->GetItemCount(); i++)
    {
        ListCtrlNonModels->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
    AddSelectedModels();

    ValidateWindow();
}

void ViewsModelsPanel::OnButton_AddSelectedClick(wxCommandEvent& event)
{
    AddSelectedModels();

    ValidateWindow();
}

void ViewsModelsPanel::OnButton_RemoveSelectedClick(wxCommandEvent& event)
{
    RemoveSelectedModels();

    ValidateWindow();
}

void ViewsModelsPanel::OnButton_RemoveAllClick(wxCommandEvent& event)
{
    for (int i = 0; i < ListCtrlModels->GetItemCount(); i++)
    {
        ListCtrlModels->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
    RemoveSelectedModels();

    ValidateWindow();
}

void ViewsModelsPanel::Initialize()
{
    if (_seqData->NumFrames() == 0) return;
    PopulateViews();
    PopulateModels();

    // ensure the selected view exists ... this can happen if the user creates views but doesnt save them
    if (_sequenceViewManager->GetView(_sequenceElements->GetCurrentView()) == nullptr) _sequenceElements->SetCurrentView(0);

    SelectView(_sequenceElements->GetViewName(_sequenceElements->GetCurrentView()));
    Fit();
    FitInside();
    ValidateWindow();
}

void ViewsModelsPanel::SetSequenceElementsModelsViews(SequenceData* seqData, SequenceElements* sequenceElements, wxXmlNode* modelsNode, wxXmlNode* modelGroupsNode, SequenceViewManager* sequenceViewManager)
{
    _sequenceElements = sequenceElements;
    _seqData = seqData;
    _models = modelsNode;
    _sequenceViewManager = sequenceViewManager;
    _modelGroups = modelGroupsNode;
}

void ViewsModelsPanel::ValidateWindow()
{
    if (ListCtrlNonModels->GetSelectedItemCount() == 0)
    {
        Button_AddSelected->Enable(false);
    }
    else
    {
        Button_AddSelected->Enable(true);
    }

    if (ListCtrlModels->GetSelectedItemCount() == 0)
    {
        Button_RemoveSelected->Enable(false);
    }
    else
    {
        Button_RemoveSelected->Enable(true);
    }

    if (ListCtrlViews->GetSelectedItemCount() == 0 || _sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW)
    {
        Button_DeleteView->Enable(false);
    }
    else
    {
        Button_DeleteView->Enable(true);
    }
}

void ViewsModelsPanel::UpdateModelsForSelectedView()
{
    int current_view = _sequenceElements->GetCurrentView();
    if (current_view != MASTER_VIEW)
    {
        wxString viewName = ListCtrlViews->GetItemText(current_view, 1);
        wxString models = "";
        for (int i = 0; i < _sequenceElements->GetElementCount(current_view); i++)
        {
            Element* elem = _sequenceElements->GetElement(i, current_view);
            if (elem->GetType() == ELEMENT_TYPE_MODEL)
            {
                if (models != "")
                {
                    models += ",";
                }
                models += elem->GetName();
            }
        }

        SequenceView* view = _sequenceViewManager->GetView(viewName.ToStdString());
        view->SetModels(models.ToStdString());
    }
    PopulateModels();
}

#pragma region View List
void ViewsModelsPanel::OnListCtrlViewsItemSelect(wxListEvent& event)
{
    int index = event.m_itemIndex;
    SelectView(ListCtrlViews->GetItemText(index, 1).ToStdString());
}

void ViewsModelsPanel::OnListCtrlItemCheck(wxCommandEvent& event)
{
    //int index = event.m_itemIndex;
    //SelectView(ListCtrlViews->GetItemText(index, 1).ToStdString());
    Element* e = (Element*)event.GetClientData();
    if (e == nullptr)
    {
        auto sv = _sequenceViewManager->GetSelectedView();

        int selected = 0;
        const int itemCount = ListCtrlViews->GetItemCount();
        for (int i = 0; i<itemCount; i++)
        {
            if (ListCtrlViews->IsChecked(i))
            {
                selected++;
            }
        }

        if (selected == 0)
        {
            SelectView(sv->GetName());
        }
        else if (selected == 2)
        {
            for (int i = 0; i < itemCount; i++)
            {
                if (ListCtrlViews->IsChecked(i))
                {
                    if (sv->GetName() == ListCtrlViews->GetItemText(i, 1))
                    {
                        ListCtrlViews->SetChecked(i, false);
                    }
                    else
                    {
                        SelectView(ListCtrlViews->GetItemText(i, 1).ToStdString());
                    }
                }
            }
        }
        else
        {
            bool found = false;
            for (int i = 0; i < itemCount; i++)
            {
                if (ListCtrlViews->IsChecked(i))
                {
                    if (!found)
                    {
                        found = true;
                        SelectView(ListCtrlViews->GetItemText(i, 1).ToStdString());
                    }
                    else
                    {
                        ListCtrlViews->SetChecked(i, false);
                    }
                }
            }
        }
    }
    else
    {
        e->SetVisible(!e->GetVisible());
    }

    MarkViewsChanged();

    // Update Grid
    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

void ViewsModelsPanel::SelectView(const std::string& view)
{
    if (_seqData->NumFrames() == 0) return;

    ListCtrlViews->SetChecked(_sequenceElements->GetCurrentView(), false);
    int selected_view = GetViewIndex(view);
    if (selected_view > 0)
    {
        std::string modelsString = _sequenceElements->GetViewModels(view);
        _sequenceElements->AddMissingModelsToSequence(modelsString);
        _sequenceElements->PopulateView(modelsString, selected_view);
    }
    _sequenceViewManager->SetSelectedView(selected_view);
    _sequenceElements->SetCurrentView(selected_view);
    _sequenceElements->SetTimingVisibility(view);
    PopulateModels();
    ListCtrlViews->SetChecked(_sequenceElements->GetCurrentView(), true);
    _mainViewsChoice->SetStringSelection(view);

    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

int ViewsModelsPanel::GetViewIndex(const wxString& name)
{
    return _sequenceViewManager->GetViewIndex(name.ToStdString());
}

void ViewsModelsPanel::MarkViewsChanged()
{
    wxCommandEvent eventRgbEffects(EVT_RGBEFFECTS_CHANGED);
    wxPostEvent(GetParent(), eventRgbEffects);
}

// This gives the panel a pointer to the view choice box on the sequencer tab
void ViewsModelsPanel::SetViewChoice(wxChoice* choice)
{
    _mainViewsChoice = choice;

    // this directs events from sequencer tab
    _mainViewsChoice->Connect(wxEVT_CHOICE, (wxObjectEventFunction)&ViewsModelsPanel::OnViewSelect, nullptr, this);
}

void ViewsModelsPanel::OnViewSelect(wxCommandEvent &event) {
    SelectView(_mainViewsChoice->GetString(_mainViewsChoice->GetSelection()).ToStdString());
    ValidateWindow();
}

void ViewsModelsPanel::PopulateViews()
{
    ListCtrlViews->ClearAll();
    if (_mainViewsChoice != nullptr) {
        _mainViewsChoice->Clear();
    }

    wxListItem col0;
    col0.SetId(0);
    col0.SetText(_(""));
    col0.SetWidth(30);
    ListCtrlViews->InsertColumn(0, col0);

    wxListItem col1;
    col1.SetId(1);
    col1.SetText(_("View"));
    col1.SetWidth(130);
    ListCtrlViews->InsertColumn(1, col1);

    _numViews = 0;
    auto views = _sequenceViewManager->GetViews();
    for (auto it = views.begin(); it != views.end(); ++it)
    {
        bool isChecked = _sequenceViewManager->GetSelectedView() == *it;
        AddViewToList((*it)->GetName(), isChecked);
        if (isChecked)
        {
            ListCtrlViews->SetChecked(0, false);
        }
    }

    _mainViewsChoice->SetSelection(_sequenceViewManager->GetSelectedViewIndex());
}

void ViewsModelsPanel::AddViewToList(const wxString& viewName, bool isChecked)
{
    wxListItem li;
    li.SetId(_numViews);
    ListCtrlViews->InsertItem(li);
    ListCtrlViews->SetItem(_numViews, 1, viewName);
    ListCtrlViews->SetChecked(_numViews, isChecked);
    _numViews++;

    _mainViewsChoice->Append(viewName);
}

void ViewsModelsPanel::OnButton_AddViewClick(wxCommandEvent& event)
{
    if (_seqData->NumFrames() == 0) return;
    wxTextEntryDialog dialog(this, _("Enter Name for View"), _("Create View"));
    int DlgResult = dialog.ShowModal();;
    if (DlgResult != wxID_OK) return;
    std::string viewName = dialog.GetValue().Trim().ToStdString();

    _sequenceViewManager->AddView(viewName);

    AddViewToList(viewName, true);
    _sequenceElements->AddView(viewName);
    SelectView(viewName);
    MarkViewsChanged();
    PopulateViews();
    ValidateWindow();
}

void ViewsModelsPanel::DeleteSelectedView()
{
    if (_seqData->NumFrames() == 0) return;

    int result = wxMessageBox("Are you sure you want to delete this View?", "Confirm Deletion", wxOK | wxCANCEL | wxCENTER);
    if (result != wxOK) return;

    ListCtrlViews->Freeze();
    long itemIndex = -1;

    for (;;) {
        itemIndex = ListCtrlViews->GetNextItem(itemIndex,
            wxLIST_NEXT_ALL,
            wxLIST_STATE_SELECTED);

        if (itemIndex == -1) break;

        // Got a selected item so handle it
        if (itemIndex > 0)  // don't delete master view
        {
            _sequenceElements->RemoveView(itemIndex);
            wxString name = _sequenceElements->GetViewName(itemIndex);
            _sequenceViewManager->DeleteView(name.ToStdString());

            ListCtrlViews->DeleteItem(itemIndex);
            _numViews--;
            break;
        }
        itemIndex = -1; // reset to delete next item which may have same index
    }
    ListCtrlViews->Thaw();
    ListCtrlViews->Refresh();
    _sequenceElements->SetCurrentView(MASTER_VIEW);
    SelectView("Master View");
    MarkViewsChanged();
    PopulateViews();
    ValidateWindow();
}

void ViewsModelsPanel::OnButton_DeleteViewClick(wxCommandEvent& event)
{
    DeleteSelectedView();
}
#pragma endregion

void ViewsModelsPanel::OnResize(wxSizeEvent& event)
{
    wxSize s = GetSize();
    Panel_Sizer->SetSize(s);
    Panel_Sizer->SetMinSize(s);
    Panel_Sizer->SetMaxSize(s);
    Panel_Sizer->Refresh();

    ScrolledWindowViewsModels->SetSize(s);
    ScrolledWindowViewsModels->SetMinSize(s);
    ScrolledWindowViewsModels->SetMaxSize(s);

    ScrolledWindowViewsModels->FitInside();
    ScrolledWindowViewsModels->SetScrollRate(5, 5);
    ScrolledWindowViewsModels->Refresh();
}

void ViewsModelsPanel::OnLeftUp(wxMouseEvent& event)
{
    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

#pragma region Non Models

void ViewsModelsPanel::AddTimingToNotList(Element* timing)
{
    if (timing != nullptr)
    {
        wxListItem li;
        li.SetId(_numNonModels);
        ListCtrlNonModels->InsertItem(li, 0);
        ListCtrlNonModels->SetItemPtrData(_numNonModels, (wxUIntPtr)timing);
        ListCtrlNonModels->SetItem(_numNonModels, 1, timing->GetName());

        _numNonModels++;
    }
}

void ViewsModelsPanel::AddModelToNotList(Element* model)
{
    if (model != nullptr)
    {
        wxListItem li;
        li.SetId(_numNonModels);
        if (IsModelAGroup(model->GetName()))
        {
            ListCtrlNonModels->InsertItem(li, 1);
        }
        else
        {
            ListCtrlNonModels->InsertItem(li, -1);
        }
        ListCtrlNonModels->SetItemPtrData(_numNonModels, (wxUIntPtr)model);
        ListCtrlNonModels->SetItem(_numNonModels, 1, model->GetName());

        _numNonModels++;
    }
}

void ViewsModelsPanel::OnListCtrlNonModelsItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void ViewsModelsPanel::OnListCtrlNonModelsBeginDrag(wxListEvent& event)
{
    if (ListCtrlNonModels->GetSelectedItemCount() == 0) return;

    _dragRowModel = false;
    _dragRowNonModel = true;

    wxString drag = "NonModel";
    for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListCtrlNonModels, i))
        {
            drag += "," + ListCtrlNonModels->GetItemText(i, 1);
        }
    }

    wxTextDataObject my_data(drag);
    VMDropSource dragSource(this, false, true);
    dragSource.SetData(my_data);
    dragSource.DoDragDrop(wxDrag_DefaultMove);
    SetCursor(wxCURSOR_ARROW);

    ValidateWindow();
}

void ViewsModelsPanel::OnListCtrlNonModelsKeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_RIGHT)
    {
        AddSelectedModels();
    }
    ValidateWindow();
}

#pragma endregion Non Models

#pragma region Models

int ViewsModelsPanel::GetTimingCount()
{
    int timings = 0;

    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i)
    {
        Element* t = (Element*)ListCtrlModels->GetItemData(i);
        if (t->GetType() == ELEMENT_TYPE_TIMING)
        {
            timings++;
        }
        else
        {
            break;
        }
    }

    return timings;
}

void ViewsModelsPanel::AddTimingToList(Element* timing)
{
    if (timing != nullptr)
    {
        wxListItem li;
        li.SetId(_numModels);
        ListCtrlModels->InsertItem(li);
        ListCtrlModels->SetItemPtrData(_numModels, (wxUIntPtr)timing);
        ListCtrlModels->SetItem(_numModels, 2, timing->GetName());
        ListCtrlModels->SetChecked(_numModels, timing->GetVisible());
        ListCtrlModels->SetItemColumnImage(_numModels, 1, TIMING_IMAGE);
        _numModels++;
    }
}

void ViewsModelsPanel::AddModelToList(Element* model)
{
    if (model != nullptr)
    {
        wxListItem li;
        li.SetId(_numModels);
        ListCtrlModels->InsertItem(li);
        ListCtrlModels->SetItemPtrData(_numModels, (wxUIntPtr)model);
        ListCtrlModels->SetItem(_numModels, 2, model->GetName());
        ListCtrlModels->SetChecked(_numModels, model->GetVisible());
        // Need to solve this ... I think we only want images for groups.
        if (IsModelAGroup(model->GetName()))
        {
            ListCtrlModels->SetItemColumnImage(_numModels, 1, MODEL_IMAGE);
        }
        _numModels++;
    }
}

void ViewsModelsPanel::OnListCtrlViewsKeyDown(wxListEvent& event)
{
    auto key = event.GetKeyCode();
    if ((key == WXK_DELETE || key == WXK_NUMPAD_DELETE) && Button_DeleteView->IsEnabled())
    {
        DeleteSelectedView();
    }
}

#pragma endregion Models

#pragma region Drag and Drop

wxDragResult MyTextDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    static wxLongLong lastTime = wxGetUTCTimeMillis();

    if (wxGetUTCTimeMillis() - lastTime < 300)
    {
        // too soon to scroll again
    }
    else
    {
        if (_type == "Model" && _list->GetItemCount() > 0)
        {
            wxRect rect;
            _list->GetItemRect(0, rect);
            int itemSize = rect.GetHeight();

            if (y < 2 * itemSize)
            {
                // scroll up
                if (_list->GetTopItem() > 0)
                {
                    lastTime = wxGetUTCTimeMillis();
                    _list->EnsureVisible(_list->GetTopItem()-1);
                }
            }
            else if (y > _list->GetRect().GetHeight() - itemSize)
            {
                // scroll down
                int flags = wxLIST_HITTEST_ONITEM;
                int lastItem = _list->HitTest(wxPoint(x, y), flags, nullptr);
                if (lastItem >= 0 && lastItem < _list->GetItemCount())
                {
                    _list->EnsureVisible(lastItem+1);
                    lastTime = wxGetUTCTimeMillis();
                }
            }
        }
    }

    return wxDragMove;
}

bool MyTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    long mousePos = x;
    mousePos = mousePos << 16;
    mousePos += y;
    wxCommandEvent event(EVT_VMDROP);
    event.SetString(data); // this is the dropped string
    event.SetExtraLong(mousePos); // this is the mouse position packed into a long

    wxArrayString parms = wxSplit(data, ',');

    if (parms[0] == "NonModel")
    {
        if (_type == "Model")
        {
            event.SetInt(0);
            wxPostEvent(_owner, event);
            return true;
        }
    }
    else if (parms[0] == "Model")
    {
        if (_type == "Model")
        {
            event.SetInt(1);
            wxPostEvent(_owner, event);
            return true;
        }
        else if (_type == "NonModel")
        {
            event.SetInt(2);
            wxPostEvent(_owner, event);
            return true;
        }
    }

    return false;
}

void ViewsModelsPanel::OnDrop(wxCommandEvent& event)
{
    wxArrayString parms = wxSplit(event.GetString(), ',');
    int x = event.GetExtraLong() >> 16;
    int y = event.GetExtraLong() & 0xFFFF;

    switch(event.GetInt())
    {
    case 0:
        // Non model dropped into models (an add)
        {
            int flags = wxLIST_HITTEST_ONITEM;
            long index = ListCtrlModels->HitTest(wxPoint(x, y), flags, nullptr);
            AddSelectedModels(index);
        }
        break;
    case 1:
        // Model dropped into models (a reorder)
        {
            bool itemsMoved = false;

            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            int flags = wxLIST_HITTEST_ONITEM;
            long index = ListCtrlModels->HitTest(wxPoint(x, y), flags, nullptr);

            int selcnt = 0;
            for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i)
            {
                if (IsItemSelected(ListCtrlModels, i) && ((Element*)ListCtrlModels->GetItemData(i))->GetType() != ELEMENT_TYPE_TIMING)
                {
                    int from = i - GetTimingCount();
                    int to = -1;
                    // we are moving this one
                    itemsMoved = true;
                    if (index == -1)
                    {
                        // moving to the end
                        to = _sequenceElements->GetElementCount(_sequenceViewManager->GetSelectedViewIndex());
                    }
                    else
                    {
                        to = index + selcnt - GetTimingCount();
                    }

                    // not sure why we need to do this with the master only
                    if (_sequenceViewManager->GetSelectedViewIndex() == 0)
                    {
                        from += GetTimingCount();
                        to += GetTimingCount();
                    }

                    if (from < to)
                    {
                        from -= selcnt;
                    }

                    _sequenceElements->MoveSequenceElement(from, to, _sequenceViewManager->GetSelectedViewIndex());
                    DeselectItem(ListCtrlModels, i);

                    selcnt++;
                }
            }

            if (itemsMoved)
            {
                MarkViewsChanged();
                UpdateModelsForSelectedView();
                PopulateModels();
                _xlFrame->DoForceSequencerRefresh();
            }
        }
        break;
    case 2:
        // Model dropped into non model (a remove)
        RemoveSelectedModels();
        break;
    default:
        break;
    }
}

#pragma endregion Drag and Drop

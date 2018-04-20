///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 20 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __ROTATIONCTRLUI_H__
#define __ROTATIONCTRLUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class PreferencesDialogBase
///////////////////////////////////////////////////////////////////////////////
class PreferencesDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxCheckBox* m_cbManualRotate;
		wxCheckBox* m_cbManualTilt;
		wxCheckBox* m_cbNorthUp;
		wxCheckBox* m_cbSouthUp;
		wxCheckBox* m_cbCourseUp;
		wxCheckBox* m_cbHeadingUp;
		wxCheckBox* m_cbRouteUp;
		wxCheckBox* m_cbWindUp;
		wxStaticText* m_staticText28;
		wxSpinCtrlDouble* m_sUpdateRate;
		wxStaticText* m_staticText29;
		wxStaticText* m_staticText30;
		wxSpinCtrl* m_sFilterSeconds;
		wxStaticText* m_staticText31;
		wxStaticText* m_staticText7;
		wxSpinCtrlDouble* m_sMaxSlewRate;
		wxStaticText* m_staticText8;
		wxStaticText* m_staticText5;
		wxSpinCtrl* m_sRotationOffset;
		wxStaticText* m_staticText6;
		wxCheckBox* m_cbRotationKeys;
		wxButton* m_button6;
		wxButton* m_button7;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnConfigure( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnConfigure( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnConfigureKeys( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnInformation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAboutAuthor( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		PreferencesDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Rotation Control Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~PreferencesDialogBase();
	
};

#endif //__ROTATIONCTRLUI_H__

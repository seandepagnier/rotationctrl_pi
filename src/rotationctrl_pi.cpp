/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  rotation control Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2015 by Sean D'Epagnier                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include <wx/wx.h>

#include "rotationctrl_pi.h"
#include "PreferencesDialog.h"
#include "icons.h"


double heading_resolve(double degrees)
{
    while(degrees < -180)
        degrees += 360;
    while(degrees >= 180)
        degrees -= 360;
    return degrees;
}

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new rotationctrl_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}

//-----------------------------------------------------------------------------
//
//    Rotationctrl PlugIn Implementation
//
//-----------------------------------------------------------------------------

rotationctrl_pi::rotationctrl_pi(void *ppimgr)
    : opencpn_plugin_113(ppimgr)
{
    // Create the PlugIn icons
    initialize_images();
    m_lastfix.Lat = NAN;
    m_lasttimerfix.Lat = NAN;
    m_sog = m_cog = NAN;
}

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

int rotationctrl_pi::Init(void)
{
    AddLocaleCatalog( _T("opencpn-rotationctrl_pi") );

    m_currenttool = 0;
        
    m_leftclick_tool_ids[MANUAL_CCW] = InsertPlugInTool
        (_T(""), _img_ccw, _img_ccw, wxITEM_NORMAL,
         _("Rotate CCW"), _T(""), NULL, TOOL_POSITION, 0, this);

    m_leftclick_tool_ids[MANUAL_CW] = InsertPlugInTool
        (_T(""), _img_cw, _img_cw, wxITEM_NORMAL,
         _("Rotate CW"), _T(""), NULL, TOOL_POSITION, 0, this);

    m_leftclick_tool_ids[NORTH_UP] = InsertPlugInTool
        (_T(""), _img_northup, _img_northup, wxITEM_NORMAL,
         _("North Up"), _T(""), NULL, TOOL_POSITION, 0, this);

    m_leftclick_tool_ids[SOUTH_UP] = InsertPlugInTool
        (_T(""), _img_southup, _img_southup, wxITEM_NORMAL,
         _("South Up"), _T(""), NULL, TOOL_POSITION, 0, this);

    m_leftclick_tool_ids[COURSE_UP] = InsertPlugInTool
        (_T(""), _img_courseup, _img_southup, wxITEM_NORMAL,
         _("Course Up"), _T(""), NULL, TOOL_POSITION, 0, this);

    m_leftclick_tool_ids[HEADING_UP] = InsertPlugInTool
        (_T(""), _img_headingup, _img_headingup, wxITEM_NORMAL,
         _("Heading Up"), _T(""), NULL, TOOL_POSITION, 0, this);

    m_leftclick_tool_ids[ROUTE_UP] = InsertPlugInTool
        (_T(""), _img_routeup, _img_routeup, wxITEM_NORMAL,
         _("Route Up"), _T(""), NULL, TOOL_POSITION, 0, this);

    m_leftclick_tool_ids[WIND_UP] = InsertPlugInTool
        (_T(""), _img_windup, _img_windup, wxITEM_NORMAL,
         _("Wind Up"), _T(""), NULL, TOOL_POSITION, 0, this);

    LoadConfig(); //    And load the configuration items
    
    m_Timer.Connect(wxEVT_TIMER, wxTimerEventHandler
                    ( rotationctrl_pi::OnTimer ), NULL, this);
    m_Timer.Start(m_filter_msecs, true);

    return (WANTS_TOOLBAR_CALLBACK    |
            WANTS_PREFERENCES         |
            WANTS_ONPAINT_VIEWPORT |
            WANTS_NMEA_SENTENCES |
            WANTS_NMEA_EVENTS         |
            WANTS_CONFIG);
}

bool rotationctrl_pi::DeInit(void)
{
    SaveConfig();

    for(int i=0; i<NUM_ROTATION_TOOLS; i++)
        RemovePlugInTool(m_leftclick_tool_ids[i]);

    return true;
}

int rotationctrl_pi::GetAPIVersionMajor()
{
    return MY_API_VERSION_MAJOR;
}

int rotationctrl_pi::GetAPIVersionMinor()
{
    return MY_API_VERSION_MINOR;
}

int rotationctrl_pi::GetPlugInVersionMajor()
{
    return PLUGIN_VERSION_MAJOR;
}

int rotationctrl_pi::GetPlugInVersionMinor()
{
    return PLUGIN_VERSION_MINOR;
}

wxBitmap *rotationctrl_pi::GetPlugInBitmap()
{
    return _img_rotation;
}

wxString rotationctrl_pi::GetCommonName()
{
    return _("Rotation Control");
}

wxString rotationctrl_pi::GetShortDescription()
{
    return _("Rotation Control PlugIn for OpenCPN");
}

wxString rotationctrl_pi::GetLongDescription()
{
    return _("Rotation Control PlugIn for OpenCPN\n\
Advanced control of the viewport rotation angle.");
}

int rotationctrl_pi::GetToolbarToolCount(void)
{
    return NUM_ROTATION_TOOLS;
}

void rotationctrl_pi::SetColorScheme(PI_ColorScheme cs)
{
}

void rotationctrl_pi::OnToolbarToolCallback(int id)
{
    m_currenttool = 0;
    for(int i=0; i<NUM_ROTATION_TOOLS; i++)
        if(m_leftclick_tool_ids[i] == id) {
            switch(i) {
            case MANUAL_CCW:
                m_vp.rotation += 3 * M_PI / 180;
                SetViewport(m_vp);
                break;
            case MANUAL_CW:
                m_vp.rotation -= 3 * M_PI / 180;
                SetViewport(m_vp);
                break;
            case NORTH_UP:
                m_vp.rotation = 0;
                SetViewport(m_vp);
                break;
            case SOUTH_UP:
                m_vp.rotation = M_PI;
                SetViewport(m_vp);
                break;
            case COURSE_UP:
            case HEADING_UP:
            case ROUTE_UP:
            case WIND_UP:
                SetToolbarItemState( id, true );
                m_currenttool = i;
                break;
            }
        } else
            SetToolbarItemState( m_leftclick_tool_ids[i], false );
}

void rotationctrl_pi::OnTimer( wxTimerEvent & )
{
    /* calculate course and speed over ground from gps */
    double dt = m_lastfix.FixTime - m_lasttimerfix.FixTime;
    if(!isnan(m_lastfix.Lat) && !isnan(m_lasttimerfix.Lat) && dt > 0) {
        /* this way helps avoid surge speed from gps from surfing waves etc... */
        double cog, sog;
        DistanceBearingMercator_Plugin(m_lastfix.Lat, m_lastfix.Lon,
                                       m_lasttimerfix.Lat, m_lasttimerfix.Lon, &cog, &sog);
        sog *= 3600.0 / dt;

        if(isnan(m_cog))
            m_cog = cog, m_sog = sog;
        else {
            m_cog = m_filter_lp*cog + (1-m_filter_lp)*m_cog;
            m_sog = m_filter_lp*sog + (1-m_filter_lp)*m_sog;
        }
    } else
        m_sog = m_cog = NAN;
    
    m_lasttimerfix = m_lastfix;

    switch(m_currenttool) {
    case COURSE_UP:   m_vp.rotation = m_cog;  break;
    case HEADING_UP:  m_vp.rotation = 0; break; // incomplete
    case ROUTE_UP:    m_vp.rotation = 0; break; // incomplete
    case WIND_UP:     m_vp.rotation = 0; break; // incomplete
    }

    if(m_currenttool)
        SetViewport(m_vp);

    m_Timer.Start(m_filter_msecs, true);
}

bool rotationctrl_pi::LoadConfig(void)
{
    wxFileConfig *pConf = GetOCPNConfigObject();

    if(!pConf)
       return false;

    pConf->SetPath ( _T( "/Settings/RotationCtrl" ) );

    SetToolbarToolViz(m_leftclick_tool_ids[MANUAL_CCW], pConf->Read( _T ( "Manual" ), 1L));
    SetToolbarToolViz(m_leftclick_tool_ids[MANUAL_CW], pConf->Read( _T ( "Manual" ), 1L));
    SetToolbarToolViz(m_leftclick_tool_ids[NORTH_UP], pConf->Read( _T ( "NorthUp" ), 1L));
    SetToolbarToolViz(m_leftclick_tool_ids[SOUTH_UP], pConf->Read( _T ( "SouthUp" ), 0L));
    SetToolbarToolViz(m_leftclick_tool_ids[COURSE_UP], pConf->Read( _T ( "CourseUp" ), 1L));
    SetToolbarToolViz(m_leftclick_tool_ids[HEADING_UP], pConf->Read( _T ( "HeadingUp" ), 0L));
    SetToolbarToolViz(m_leftclick_tool_ids[ROUTE_UP], pConf->Read( _T ( "RouteUp" ), 0L));
    SetToolbarToolViz(m_leftclick_tool_ids[WIND_UP], pConf->Read( _T ( "WindUp" ), 0L));

    m_filter_msecs = 1000.0 * pConf->Read( _T ( "UpdatePeriod" ), 5.0);
    m_filter_lp = 1.0 / pConf->Read( _T ( "FilterSeconds" ), 10.0);
    
    return true;
}

bool rotationctrl_pi::SaveConfig(void)
{
    wxFileConfig *pConf = GetOCPNConfigObject();

    if(!pConf)
        return false;

    pConf->SetPath ( _T ( "/Settings/Rotationctrl" ) );

    return true;
}

void rotationctrl_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix)
{
    if(pfix.FixTime && pfix.nSats)
        m_LastFixTime = wxDateTime::Now();

    m_lastfix = pfix;
}

void rotationctrl_pi::ShowPreferencesDialog( wxWindow* parent )
{
    PreferencesDialog dlg(parent);
    dlg.ShowModal();

    LoadConfig();
}

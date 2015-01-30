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

#include "jsonreader.h"
#include "jsonwriter.h"

#include "rotationctrl_pi.h"
#include "PreferencesDialog.h"
#include "icons.h"

static double heading_resolve(double radians)
{
    while(radians < 0)
        radians += 2*M_PI;
    while(radians >= 2*M_PI)
        radians -= 2*M_PI;
    return radians;
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
    m_vp.rotation = 0;
    Reset();
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

    return (WANTS_TOOLBAR_CALLBACK |
            WANTS_PREFERENCES      |
            WANTS_ONPAINT_VIEWPORT |
            WANTS_NMEA_SENTENCES   |
            WANTS_NMEA_EVENTS      |
            WANTS_CONFIG           |
            WANTS_PLUGIN_MESSAGING
        );
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
    for(int i=0; i<NUM_ROTATION_TOOLS; i++)
        if(m_leftclick_tool_ids[i] == id) {
            switch(i) {
            case NORTH_UP:
                SetCanvasRotation(0);
                m_currenttool = 0;
                break;
            case SOUTH_UP:
                SetCanvasRotation(M_PI);
                m_currenttool = 0;
                break;
            case COURSE_UP:
            case HEADING_UP:
            case ROUTE_UP:
            case WIND_UP:
                if(m_currenttool == i) {
                    m_currenttool = 0;
                    SetToolbarItemState( id, false );
                    m_Timer.Stop();
                } else {
                    Reset();
                    SetToolbarItemState( id, true );
                    m_currenttool = i;
                    m_Timer.Start(1, true); // start right away
                }
                break;
            }
        } else
            SetToolbarItemState( m_leftclick_tool_ids[i], false );
}

void rotationctrl_pi::OnToolbarToolDownCallback(int id)
{
    for(int i=0; i<NUM_ROTATION_TOOLS; i++)
        if(m_leftclick_tool_ids[i] == id) {
            switch(i) {
            case MANUAL_CCW:
                m_rotation_dir = -1;
                break;
            case MANUAL_CW:
                m_rotation_dir = 1;
                break;
            default: return;
            }
        }

    m_last_rotation_time = wxDateTime::UNow();
    RequestRefresh(GetOCPNCanvasWindow());
}

void rotationctrl_pi::OnToolbarToolUpCallback(int id)
{
    m_rotation_dir = 0;
}

void rotationctrl_pi::OnTimer( wxTimerEvent & )
{
//    double dt = m_lastfix.FixTime - m_lasttimerfix.FixTime;

    m_cog = FilterAngle(deg2rad(m_lastfix.Cog), m_cog);
    m_sog = FilterSpeed(m_lastfix.Sog, m_sog);
    
//    m_lasttimerfix = m_lastfix;

    switch(m_currenttool) {
    case COURSE_UP:   m_vp.rotation = -m_cog;  break;
    case HEADING_UP:  m_vp.rotation = -m_heading; break;
    case ROUTE_UP:
    {
        double lastlat = m_routewaypoint.m_lat;
        double lastlon = m_routewaypoint.m_lon;
        if(GetSingleWaypoint( m_routeguid, &m_routewaypoint )) {
            if(lastlat != m_routewaypoint.m_lat ||
               lastlon != m_routewaypoint.m_lon)
                Reset();

            double route_heading;
            DistanceBearingMercator_Plugin
                (m_routewaypoint.m_lat, m_routewaypoint.m_lon,
                 m_lastfix.Lat, m_lastfix.Lon,
                 &route_heading, NULL);

            route_heading = deg2rad(route_heading);
            
            if(isnan(m_route_heading))
                m_route_heading = route_heading;

            m_route_heading = FilterAngle(route_heading, m_route_heading);
        }

        m_vp.rotation = -m_route_heading;
    } break;
    case WIND_UP:     m_vp.rotation = -m_truewind; break;
    default: return;
    }

    m_vp.rotation += deg2rad(m_rotation_offset);

    if(isnan(m_vp.rotation))
        return;

    SetCanvasRotation(m_vp.rotation);

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

    double filter_seconds = 5;
    pConf->Read( _T ( "UpdatePeriod" ), _T("5")).ToDouble(&filter_seconds);
    if(filter_seconds < .05) {
        wxMessageDialog mdlg(NULL, _("Invalid update period, defaulting to 5 seconds"),
                             _("Rotation Control Information"), wxOK | wxICON_INFORMATION);
        mdlg.ShowModal();
        filter_seconds = 5;
    }

    m_filter_msecs = 1000.0 * filter_seconds;
    m_filter_lp = m_filter_msecs / 1000.0 / pConf->Read( _T ( "FilterSeconds" ), 10.0);
    m_rotation_offset = pConf->Read( _T ( "RotationOffset" ), 0L);
    
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

void rotationctrl_pi::SetCurrentViewPort(PlugIn_ViewPort &vp)
{
    if(heading_resolve(m_vp.rotation - vp.rotation) > .001) {
        for(int i=0; i<NUM_ROTATION_TOOLS; i++)
            SetToolbarItemState( m_leftclick_tool_ids[i], false );

        m_Timer.Stop();
        m_currenttool = 0;
    }

    m_vp = vp;

    if(!m_rotation_dir)
        return;

    wxDateTime now = wxDateTime::UNow();
    long dt = 0;
    if(m_last_rotation_time.IsValid())
        dt = (now - m_last_rotation_time).GetMilliseconds().ToLong();

    if(dt > 500) /* if we are running very slow, don't integrate too fast */
        dt = 500;
    
    int rotation_speed = wxGetMouseState().AltDown() ? 6 : 60;
    
    SetCanvasRotation(m_vp.rotation + m_rotation_dir * rotation_speed *
                      M_PI / 180 * dt / 1000.0);
    m_last_rotation_time = now;
    RequestRefresh(GetOCPNCanvasWindow());
}

void rotationctrl_pi::SetNMEASentence( wxString &sentence )
{
    if(!m_currenttool)
        return;

    m_NMEA0183 << sentence;

    if( !m_NMEA0183.PreParse() )
        return;

    if( m_currenttool == HEADING_UP && m_NMEA0183.LastSentenceIDReceived == _T("HDT") ) {
        if( m_NMEA0183.Parse() ) {
            if( !wxIsNaN(m_NMEA0183.Hdt.DegreesTrue) )
                m_heading = FilterAngle(m_NMEA0183.Hdt.DegreesTrue, m_heading);
        }
    }
    // NMEA 0183 standard Wind Direction and Speed, with respect to north.
    else if( m_currenttool == WIND_UP && m_NMEA0183.LastSentenceIDReceived == _T("MWD") ) {
        if( m_NMEA0183.Parse() ) {
            // Option for True vs Magnetic
            wxString windunit;
            double truewind = 0;
            if( m_NMEA0183.Mwd.WindAngleTrue < 999. ) { //if WindAngleTrue is available, use it ...
                truewind = m_NMEA0183.Mwd.WindAngleTrue;
            } else if( m_NMEA0183.Mwd.WindAngleMagnetic < 999. ) { //otherwise try WindAngleMagnetic ...
                // TODO: use wmm plugin to compensate to true wind
                truewind = m_NMEA0183.Mwd.WindAngleMagnetic;
            }

            m_truewind = FilterAngle(truewind, m_truewind);
        }
    }

    /* maybe true wind is not available?  we could use this along with heading from nmea or computed from gps?
       can use wmm plugin if only magnetic data is available as weell
       in the case of apparent wind, we need to compensate from speed as well (either water or gps), */

#if 0 // Not yet implemented

        // NMEA 0183 standard Wind Speed and Angle, in relation to the vessel's bow/centerline.
        else if( m_NMEA0183.LastSentenceIDReceived == _T("MWV") ) {
            if( m_NMEA0183.Parse() ) {
                if( m_NMEA0183.Mwv.IsDataValid == NTrue ) {
                    m_NMEA0183.Mwv.WindAngle;
                }
            }
        }

        /* NMEA 0183 Relative (Apparent) Wind Speed and Angle. Wind angle in relation
         * to the vessel's heading, and wind speed measured relative to the moving vessel. */
        else if( m_NMEA0183.LastSentenceIDReceived == _T("VWR") ) {
            if( m_NMEA0183.Parse() ) {

                    wxString awaunit;
                    awaunit = m_NMEA0183.Vwr.DirectionOfWind == Left ? _T("\u00B0L") : _T("\u00B0R");
                    m_NMEA0183.Vwr.WindDirectionMagnitude;
                }
            }
        }
        /* NMEA 0183 True wind angle in relation to the vessel's heading, and true wind
         * speed referenced to the water. True wind is the vector sum of the Relative
         * (apparent) wind vector and the vessel's velocity vector relative to the water along
         * the heading line of the vessel. It represents the wind at the vessel if it were
         * stationary relative to the water and heading in the same direction. */
        else if( m_NMEA0183.LastSentenceIDReceived == _T("VWT") ) {
            if( m_NMEA0183.Parse() ) {
                    vwtunit = m_NMEA0183.Vwt.DirectionOfWind == Left ? _T("\u00B0L") : _T("\u00B0R");
                    m_NMEA0183.Vwt.WindDirectionMagnitude;
                }
            }
        }
#endif
}

void rotationctrl_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix)
{
#if 0
    if(m_currenttool == COURSE_UP) {
        /* calculate course and speed over ground from gps */
        if(!isnan(m_lastfix.Lat) && !isnan(m_lasttimerfix.Lat)) {
            /* this way helps avoid surge speed from gps from surfing waves etc... */
            double cog, sog;
            DistanceBearingMercator_Plugin(pfix.Lat, pfix.Lon,
                                           m_lastfix.Lat, m_lastfix.Lon, &cog, &sog);
            sog *= 3600.0 / dt;
            cog *= M_PI / 180.0;
        }
    }
#endif

    if(pfix.FixTime && pfix.nSats)
        m_LastFixTime = wxDateTime::Now();

    m_lastfix = pfix;
}

void rotationctrl_pi::SetPluginMessage(wxString &message_id, wxString &message_body)
{
    wxJSONReader r;
    wxJSONValue v;

    if(message_id == _T("OCPN_WPT_ACTIVATED"))
    {
        r.Parse(message_body, &v);
        m_routeguid = v[_T("GUID")].AsString();
        Reset();
        m_Timer.Start(1, true); // start right away
    }

    if(message_id == _T("OCPN_WPT_ARRIVED"))
    {
        m_routeguid = v[_T("GUID")].AsString();
        Reset();
        m_Timer.Start(1, true); // start right away
    }
}

void rotationctrl_pi::ShowPreferencesDialog( wxWindow* parent )
{
    {
        PreferencesDialog dlg(parent);
        dlg.ShowModal();
    } // ensure preferences destructor before loadconfig

    LoadConfig();
}

double rotationctrl_pi::FilterAngle(double input, double last)
{
    if(isnan(input))
        return last;

    if(isnan(last))
        return input;

    double x = sin(input), y = cos(input);
    double lx = sin(last), ly = cos(last);

    x = m_filter_lp*x + (1-m_filter_lp)*lx;
    y = m_filter_lp*y + (1-m_filter_lp)*ly;

    return atan2(x, y);
}

double rotationctrl_pi::FilterSpeed(double input, double last)
{
    if(isnan(input))
        return last;

    if(isnan(last))
        return input;

    return m_filter_lp*input + (1-m_filter_lp)*last;
}

void rotationctrl_pi::Reset()
{
//    m_lastfix.Lat = NAN;
    m_sog = m_cog = NAN;
    m_heading = m_truewind = NAN;
    m_route_heading = NAN;
    m_rotation_dir = 0;
}

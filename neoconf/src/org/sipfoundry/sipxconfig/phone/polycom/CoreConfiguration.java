/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.setting.PatternSettingFilter;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingGroup;

/**
 * Responsible for generating ipmid.cfg
 */
public class CoreConfiguration extends ConfigurationTemplate {
    
    private static PatternSettingFilter s_callSettings = new PatternSettingFilter();
    static {
        s_callSettings.addExcludes("/call/donotdisturb.*$");
        s_callSettings.addExcludes("/call/shared.*$");
    }
    
    public CoreConfiguration(PolycomPhone phone, Endpoint endpoint) {
        super(phone, endpoint);
    }
    
    public void addContext(VelocityContext context) {
        SettingGroup endpointSettings = getEndpoint().getSettings(getPhone());
        Setting call = endpointSettings.getSetting(CALL_SETTINGS);
        context.put("call", call.list(s_callSettings));
    }
}


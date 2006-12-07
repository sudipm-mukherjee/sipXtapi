/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.device.RestartManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class PhoneTableActions extends BaseComponent {
    @Parameter(required = true)
    public abstract SelectMap getSelections();

    /**
     * If set to true we will only present actions that makes sense on the user page
     */
    @Parameter(required = false, defaultValue = "ognl:false")
    public abstract boolean getUserOnly();

    @Parameter(required = false, defaultValue = "ognl:null")
    public abstract IPropertySelectionModel getActionModel();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:profileManager")
    public abstract ProfileManager getProfileManager();

    @InjectObject(value = "spring:restartManager")
    public abstract RestartManager getRestartManager();

    public void generateProfiles() {
        Collection phoneIds = getSelections().getAllSelected();
        generateProfiles(phoneIds);
    }

    public void generateAllProfiles() {
        Collection phoneIds = getPhoneContext().getAllPhoneIds();
        generateProfiles(phoneIds);
    }

    private void generateProfiles(Collection phoneIds) {
        getProfileManager().generateProfilesAndRestart(phoneIds);
        String msg = getMessages().format("msg.success.profiles",
                Integer.toString(phoneIds.size()));
        TapestryUtils.recordSuccess(this, msg);
    }

    public void restart() {
        Collection phoneIds = getSelections().getAllSelected();
        getRestartManager().restart(phoneIds);
        String msg = getMessages().format("msg.success.restart",
                Integer.toString(phoneIds.size()));
        TapestryUtils.recordSuccess(this, msg);
    }

    public void deletePhone() {
        PhoneContext context = getPhoneContext();

        Collection ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }

        for (Iterator i = ids.iterator(); i.hasNext();) {
            Integer phoneId = (Integer) i.next();
            Phone phone = context.loadPhone(phoneId);
            context.deletePhone(phone);
        }

        String msg = getMessages().format("msg.success.delete", Integer.toString(ids.size()));
        TapestryUtils.recordSuccess(this, msg);
    }
}

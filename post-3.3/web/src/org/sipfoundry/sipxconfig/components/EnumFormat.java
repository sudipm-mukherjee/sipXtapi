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
package org.sipfoundry.sipxconfig.components;

import java.text.FieldPosition;
import java.text.Format;
import java.text.ParsePosition;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.enums.Enum;
import org.apache.hivemind.Messages;

public class EnumFormat extends Format {
    private Messages m_messages;

    private String m_prefixSeparator = ".";

    private String m_prefix = StringUtils.EMPTY;

    public void setMessages(Messages messages) {
        m_messages = messages;
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    public void setPrefixSeparator(String prefixSeparator) {
        m_prefixSeparator = prefixSeparator;
    }

    public StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos_) {
        Enum value = (Enum) obj;
        if (m_messages == null) {
            toAppendTo.append(value.getName());
        } else {
            String key = m_prefix + m_prefixSeparator + value.getName();
            toAppendTo.append(m_messages.getMessage(key));
        }
        return toAppendTo;
    }

    public Object parseObject(String source, ParsePosition pos) {
        throw new UnsupportedOperationException();
    }
}

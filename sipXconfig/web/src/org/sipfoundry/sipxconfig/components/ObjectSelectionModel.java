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
package org.sipfoundry.sipxconfig.components;

import java.util.Collection;

import ognl.Ognl;
import ognl.OgnlException;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.util.prop.OgnlUtils;

public class ObjectSelectionModel implements IPropertySelectionModel {

    private transient Object m_parsedLabelExpression;

    private transient Object m_parsedValueExpression;

    private Object[] m_objects;

    private String m_labelExpression;
    
    private String m_valueExpression;

    public void setCollection(Collection objects) {
        m_objects = objects.toArray();
    }
    
    public void setArray(Object[] objects) {
        m_objects = (Object[]) objects.clone();        
    }

    /**
     * Will run expression on each object in collection to build label text
     */
    public void setLabelExpression(String labelOgnlExpression) {
        m_labelExpression = labelOgnlExpression;
    }

    /**
     * Will run expression on each object in collection to read/write value to
     * PropertySelection value expression
     */
    public void setValueExpression(String valueExpression) {
        m_valueExpression = valueExpression;
    }

    public int getOptionCount() {
        return m_objects.length;
    }

    public Object getOption(int index) {
        if (StringUtils.isBlank(m_valueExpression)) {
            return m_objects[index];
        }
        
        parseValueExpression();

        try {
            Object objValue = Ognl.getValue(m_parsedValueExpression, m_objects[index]);
            return objValue;
        } catch (OgnlException e) {
            throw new RuntimeException(e);
        }
    }

    public String getLabel(int index) {
        // source adapted from OgnlTableColumnEvaluator

        // If no expression is given, then this is dummy column. Return something.
        if (StringUtils.isBlank(m_labelExpression)) {
            return StringUtils.EMPTY;
        }

        parseLabelExpression();

        try {
            Object objValue = Ognl.getValue(m_parsedLabelExpression, m_objects[index]);
            return safeToString(objValue);
        } catch (OgnlException e) {
            throw new RuntimeException(e);
        }
    }
        
    private synchronized void parseLabelExpression() {
        if (m_parsedLabelExpression == null) {
            m_parsedLabelExpression = OgnlUtils.getParsedExpression(m_labelExpression);
        }
    }

    private synchronized void parseValueExpression() {
        if (m_parsedValueExpression == null) {
            m_parsedValueExpression = OgnlUtils.getParsedExpression(m_valueExpression);
        }
    }

    private String safeToString(Object o) {
        return o == null ? StringUtils.EMPTY : o.toString();
    }

    /** based off StringPropertySelectionModel */
    public String getValue(int index) {
        return Integer.toString(index);
    }

    /** based off StringPropertySelectionModel */
    public Object translateValue(String value) {
        int index = Integer.parseInt(value);
        return getOption(index);
    }
}

/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import java.io.Serializable;
import java.net.URI;
import java.net.URISyntaxException;

public class AcdAudio extends AcdComponent {
    public static final String BEAN_NAME = "acdAudio";
    public static final String OBJECT_CLASS = "acd-audio";

    static final String AUDIO_NAME = "acd-audio/name";
    static final String AUDIO_URI = "acd-audio/uri";

    public AcdAudio() {
        super("sipxacd-audio.xml", OBJECT_CLASS);
    }

    public void setName(String name) {
        setSettingValue(AUDIO_NAME, name);
    }

    public void setUri(String uri) {
        setSettingValue(AUDIO_URI, uri);
    }

    public void setAudioFileName(String audioServerUrl, String fileName) {
        setName(fileName);
        try {
            URI audioServerUri = new URI(audioServerUrl);
            String path = audioServerUri.getRawPath();
            String newPath = String.format("%s/%s", path, fileName);
            URI fileUri = new URI(audioServerUri.getScheme(), audioServerUri.getHost(), newPath,
                    null);
            setUri(fileUri.toASCIIString());
        } catch (URISyntaxException e) {
            throw new RuntimeException(audioServerUrl);
        }
    }

    public Serializable getAcdServerId() {
        return null;
    }

    @Override
    public void initialize() {
    }
}

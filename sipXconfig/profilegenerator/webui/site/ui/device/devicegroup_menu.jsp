
<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>Untitled Document</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="../../style/nn.css" type="text/css">
<style type="text/css">
<!--
@import url(../../style/dms.css);
@import url(../../style/w3csheet.css);
-->
</style>
<script language="JavaScript">
<!--
function pviiClassNew(obj, new_style) { //v2.3 by PVII
  obj.className=new_style;
}

function MM_goToURL() { //v3.0
  var i, args=MM_goToURL.arguments; document.MM_returnValue = false;
  for (i=0; i<(args.length-1); i+=2) eval(args[i]+".location='"+args[i+1]+"'");
}

function reloadFrameSet(src) {

    var mfgDeviceToLoad;
    var selectedIndex = document.frmSelect.display.selectedIndex;


    if ( selectedIndex != -1 )
    {
        mfgDeviceToLoad = document.frmSelect.display.options[selectedIndex].value;
        top.mainFrame.location = "devicegroup_frameset.jsp?" + mfgDeviceToLoad;
    }
    else
    {
        alert("Please select a Manufacturer - Device Model");
        return;
    }

}

//-->
</script>
</head>

<body class="bglight" onLoad="MM_goToURL('top.frames[\'command\']','../command/details_devicegroup_cmd.html')">

<pgs:deviceGroupTabs stylesheet='<%=request.getParameter("stylesheet")%>' outputxml="false"
                    devicegroupid='<%= request.getParameter("devicegroupid")%>'
                    mfgid='<%=request.getParameter("mfgid")%>'
                    devicetypeid='<%=request.getParameter("devicetypeid")%>'
                    usertype='<%=request.getParameter("usertype")%>'
                    level="group"
                    visiblefor="device"
                    detailstype="devicegroup"/>

</body>
</html>

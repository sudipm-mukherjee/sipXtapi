<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
   response.sendRedirect( "list_all_devicegroups.jsp" );
%>
<html>
<head>
<title>Copy DeviceGroup</title></head>
<body>
<h1>Copies a DeviceGroup</h1>

<pgs:copyDeviceGroup
    sourcedevicegroupid= '<%=request.getParameter("sourcedevicegroupid")%>' />
</body>
</html>

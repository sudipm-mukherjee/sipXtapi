<%
   String userName = session.getAttribute( "user" ).toString();
   String orgName = session.getAttribute( "orgname" ).toString();
%>
<html>
<head>
<title>Status</title>
<link rel="stylesheet" href="../../style/dms.css" type="text/css">
<script language="JavaScript">
<!--
function MM_findObj(n, d) { //v3.0
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document); return x;
}
function MM_swapImage() { //v3.0
  var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for(i=0;i<(a.length-2);i+=3)
   if ((x=MM_findObj(a[i]))!=null){document.MM_sr[j++]=x; if(!x.oSrc) x.oSrc=x.src; x.src=a[i+2];}
}
function MM_swapImgRestore() { //v3.0
  var i,x,a=document.MM_sr; for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
}

function MM_preloadImages() { //v3.0
 var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();
   var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)
   if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}
}

//-->
</script>
</head>
<body class="bgdark">
<table width="400" border="0" cellspacing="3" cellpadding="0">
  <tr>
    <td class="statustitle" width="100%">
      <div align="right">User: </div>
    </td>
    <td class="statustext" width="50"><%=userName%></td>
    <td width="10">&nbsp;</td>
    <td class="statustitle" width="50">
      <div align="right">Organization: </div>
    </td>
    <td class="statustext" width="200" nowrap><%=orgName%></td>
	<td><img src="../../image/1x1green.gif" width="200" height="1" border="0"></td>
     </tr>
</table>

</body>
</html>

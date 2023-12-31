<% @LANGUAGE="VBScript" @EnableSessionState="False"  %>
<% 'Option Explicit

Function GetString(ByVal sStringName)
	If InStr(sStringName, "L_") <> 1 Then sStringName = "L_" & sStringName & "_Text"

	'' Texts
	Const L_StartRemoteAssistance_Text  		= "Iniciar a 'Assist�ncia remota'"
	Const L_ProductName_Text			= "Assist�ncia remota"
	Const L_Title1Paragraph1_Text		= "Fornecer ajuda utilizando a 'Assist�ncia remota'"
	Const L_Paragraph1_Text				= "Por vezes, a melhor forma de ajudar algu�m a corrigir um problema � demonstrar uma solu��o. A assist�ncia remota � uma liga��o segura que pode utilizar para indicar uma solu��o para um problema relativo ao computador."
	Const L_Paragraph2_Text				= "Se � a primeira vez, leia as notas que se seguem."
	Const L_StartRAMessage_Text			= "<font color=""#000000"" size=""2"">Para ligar ao computador do seu amigo, clique em</font><b><font color=""#000000"" size=""2""><b> Iniciar 'Assist�ncia remota'.</b></font></b>"
	Const L_StartRAButton_Text			= "Iniciar 'Assist�ncia remota'"
	Const L_JScriptNotAvail1_Text		= "As defini��es de seguran�a actuais do Internet Explorer est�o definidas para valores muito elevados para permitir a utiliza��o da assist�ncia remota. Para utilizar esta assist�ncia, altere as defini��es ou adicione este site como um site fidedigno."
	Const L_IEHelp_Text					= "Para obter informa��es sobre como alterar defini��es de seguran�a, consulte a ajuda do Internet Explorer."
	Const L_TicketError1_Text			= "Falta o convite de assist�ncia remota ou este n�o � v�lido. Para utilizar a assist�ncia remota, necessita de um convite v�lido."
	Const L_InstallIE_Text				= "Para aceitar um convite de assist�ncia remota, tem de instalar o Internet Explorer 5.5."
	Const L_TicketErrorMsg_Text			= "Para mais informa��es sobre a assist�ncia remota, clique em <B>Iniciar</B> no Windows XP, clique em <B>Ajuda e suporte</B> e escreva <B>assist�ncia remota</B> na caixa <B>Procurar</B>."
	
	Const L_NotesTitle_Text				= "Notas"
	Const L_NotesLine1_Text				= "Aceitar apenas convites de pessoas nas quais confia."
	
	Const L_NotesLine2_Text				= "Tanto o utilizador como a pessoa que est� a ajudar t�m de estar ligados � Internet enquanto utilizam a 'Assist�ncia remota'. Se a pessoa que enviou este convite estiver dispon�vel para aceitar o seu pedido de liga��o ao respectivo computador, poder� ver o ecr� de trabalho desse computador e conversar em tempo real."
	
	Const L_NotesLine6_Text				= "� uma liga��o segura de um �nico sentido. Em nenhum momento o propriet�rio do computador ao qual est� a ligado para poder utilizar esse computador poder� aceder ao seu computador. Se trabalhar numa rede local, os firewalls podem impedi-lo de utilizar a 'Assist�ncia remota'. Neste caso, pe�a mais informa��es ao administrador de rede."
	
	Const L_LearnXP_Text				= "Para mais informa��es sobre a 'Assist�ncia remota', clique em <b>Iniciar</b> no Windows XP, clique em <b>Ajuda e suporte</b> e escreva <b>assist�ncia remota</b> na caixa <b>Procurar</b>."
	
	Const L_ContactUsMsg_Text			= "Contacte-nos"
	

	Const L_XPNewsletterMsg_Text		= "Newsletters gr�tis"
	Const L_EmailPage_Text				= "Enviar esta p�gina por correio electr�nico"
	
	Const L_CopyRightMsg_Text			= "(C) 2001 Microsoft Corporation. Todos os direitos reservados. Termos de utiliza��o."
	
	Const L_PrivacyMsg_Text				= "Declara��o de privacidade"

	Const L_AccMsg_Text					= "Acessibilidade"

	Const L_NotIEError1_Text			= "Para aceitar um convite de 'Assist�ncia remota', tem de instalar o Internet Explorer 5.5 ou superior."

	Const L_NotXPError1_Text			= "A 'Assist�ncia remota' da Microsoft � uma funcionalidade do Windows&nbsp;XP. Para utilizar esta funcionalidade, primeiro tem de instalar o Windows&nbsp;XP."

	Const L_IELinkMsg_Text				= "Transferir a vers�o mais actual do Internet Explorer"
	Const L_XPLinkMsg_Text				= "Para mais informa��es sobre o Windows XP, consulte o "
	Const L_XPLinkMsg2_Text				= "Web site do Microsoft Windows"
	Const L_XPLinkMsg3_Text				= "."
	
	Const L_IntroMsg1_Text              = "A transferir um componente necess�rio para utilizar a 'Assist�ncia remota'. Se tiver um modem de 28.8 kbps modem, a transfer�ncia pode demorar um minuto."
    Const L_IntroMsg2_Text              = "Para executar a 'Assist�ncia remota, o 'Processamento de scripts activo' tem que ser permitido. Se depois do tempo de transfer�ncia estimado esta p�gina n�o confirmar que a transfer�ncia est� conclu�da, verifique se o processamento de scripts est� activado."
    Const L_IntroMsg3_Text              = "Para permitir o 'Processamento de scripts activo'"
    Const L_IntroMsg4_Text              = "No menu <B>Ferramentas</B> no Internet Explorer, clique em <B>Internet Options</B>."
    Const L_IntroMsg5_Text              = "Clique no separador <B>Seguran�a</B>."
    Const L_IntroMsg6_Text              = "Clique no �cone <STRONG>Internet</STRONG> e, em seguida, clique em <STRONG>N�vel personalizado</STRONG>."
    Const L_IntroMsg7_Text              = "Na sec��o de scripts, em <STRONG>Processamento de scripts activo</STRONG>, clique <STRONG>Permitir</STRONG>."
    
	' **************************************
	' Links - No need for localization
	' **************************************
	Const L_AccLink_Text				= "http://www.microsoft.com/enable/"		
	Const L_XPLink_Text					= "http://www.microsoft.com/windowsxp/default.asp"
	Const L_PrivacyLink_Text			= "http://www.microsoft.com/info/privacy.htm"
	Const L_ContactUsLink_Text			= "http://register.microsoft.com/contactus30/contactus.asp?domain=windows&amp;lcid=1033&amp;cu=http://register.microsoft.com/contactus30/contactus.asp?domain=windows"
	Const L_MailTo1_Text				= "mailto:?subject=Uma hiperliga��o da Microsoft&body=Esta hiperliga��o � do Web site do Microsoft Windows XP"
	Const L_CopyRightLink_Text			= "http://www.microsoft.com/info/cpyright.htm"
	Const L_XPNewsletterLink_Text		= "http://www.microsoft.com/windowsxp/newsletters.asp"
	Const L_LinkToIE_Text				= "http://www.microsoft.com/windows/ie/default.htm"
	'*************************************	
	
	GetString = Eval(sStringName)
End Function

%>
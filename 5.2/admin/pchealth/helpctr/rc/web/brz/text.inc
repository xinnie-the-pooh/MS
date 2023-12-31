<% @LANGUAGE="VBScript" @EnableSessionState="False"  %>
<% 'Option Explicit

Function GetString(ByVal sStringName)
	If InStr(sStringName, "L_") <> 1 Then sStringName = "L_" & sStringName & "_Text"

	'' Texts
	Const L_StartRemoteAssistance_Text  		= "Iniciar Assist�ncia remota"
	Const L_ProductName_Text			= "Assist�ncia remota"
	Const L_Title1Paragraph1_Text		= "Fornecendo ajuda sobre o uso da 'Assist�ncia remota'"
	Const L_Paragraph1_Text				= "�s vezes, a melhor maneira de ajudar algu�m a resolver um problema � demonstrar a solu��o. A 'Assist�ncia remota' � uma conex�o segura que voc� pode usar para ajudar um amigo a solucionar um problema no computador."
	Const L_Paragraph2_Text				= "Se esta for a primeira vez que voc� usa esta op��o, leia as observa��es abaixo."
	Const L_StartRAMessage_Text			= "<font color=""#000000"" size=""2"">Para se conectar ao computador de seu amigo, clique em</font><b><font color=""#000000"" size=""2""><b> Iniciar Assist�ncia remota.</b></font></b>"
	Const L_StartRAButton_Text			= "Iniciar Assist�ncia remota"
	Const L_JScriptNotAvail1_Text		= "As configura��es de seguran�a atuais do Internet Explorer s�o muito elevadas para permitir o uso da 'Assist�ncia remota'. Para usar a 'Assist�ncia remota', altere as configura��es ou adicione este site � lista de sites confi�veis."
	Const L_IEHelp_Text					= "Para obter informa��es sobre como alterar configura��es de seguran�a, consulte a ajuda do Internet Explorer."
	Const L_TicketError1_Text			= "Seu convite da 'Assist�ncia remota' est� faltando ou n�o � v�lido. Para usar a 'Assist�ncia remota', voc� precisa de um convite v�lido."
	Const L_InstallIE_Text				= "Para aceitar um convite da 'Assist�ncia remota', voc� deve instalar o Internet Explorer 5.5."
	Const L_TicketErrorMsg_Text			= "Para obter mais informa��es sobre a 'Assist�ncia remota', clique em <B>Iniciar</B> no Windows XP, clique em <B>Ajuda e suporte</B> e digite <B>Assist�ncia remota</B> na caixa <B>Pesquisar</B>."
	
	Const L_NotesTitle_Text				= "Observa��es"
	Const L_NotesLine1_Text				= "Aceitar convites somente de pessoas em quem voc� confia."
	
	Const L_NotesLine2_Text				= "Voc� e a pessoa a quem est� ajudando devem estar conectados � Internet ao usar a 'Assist�ncia remota'. Se a pessoa que lhe enviar este convite estiver dispon�vel para aceitar seu pedido de conex�o ao computador, voc� poder� ver a tela do computador de seu(sua) amigo(a) e conversar com ele(a) em tempo real."
	
	Const L_NotesLine6_Text				= "Esta � uma conex�o segura e unidirecional. A pessoa a cujo computador voc� se conectar nunca poder� usar o computador remoto para acessar o seu computador. Se voc� estiver trabalhando em uma rede local, firewalls podem impedi-lo de usar a 'Assist�ncia remota'. Nesse caso, consulte o administrador de rede para obter mais informa��es."
	
	Const L_LearnXP_Text				= "Para obter mais informa��es sobre a 'Assist�ncia remota', clique em <b>Iniciar</b> no Windows XP, clique em <b>Ajuda e suporte</b> e digite <b>Assist�ncia remota</b> na caixa <b>Pesquisar</b>."
	
	Const L_ContactUsMsg_Text			= "Entre em contato conosco"
	

	Const L_XPNewsletterMsg_Text		= "Boletins gratuitos"
	Const L_EmailPage_Text				= "Envie esta p�gina por email"
	
	Const L_CopyRightMsg_Text			= "(C) 2001 Microsoft Corporation. Todos os direitos reservados. Termos de uso."
	
	Const L_PrivacyMsg_Text				= "Declara��o de privacidade"

	Const L_AccMsg_Text					= "Acessibilidade"

	Const L_NotIEError1_Text			= "Para aceitar um convite de 'Assist�ncia remota', voc� deve instalar o Internet Explorer 5.5 ou superior."

	Const L_NotXPError1_Text			= "A 'Assist�ncia remota' da Microsoft � um recurso do Windows&nbsp;XP. Para usar a 'Assist�ncia remota', primeiro voc� deve instalar o Windows&nbsp;XP."

	Const L_IELinkMsg_Text				= "Fazer o download da vers�o mais recente do Internet Explorer"
	Const L_XPLinkMsg_Text				= "Para obter mais informa��es sobre o Windows XP, v� para "
	Const L_XPLinkMsg2_Text				= "o site do Microsoft Windows na Web"
	Const L_XPLinkMsg3_Text				= "."
	
	Const L_IntroMsg1_Text              = "� necess�rio utilizar a 'Assist�ncia remota' para fazer o download de um componente. Com um modem de 28,8 kbps, o download levar� 1 minuto."
    Const L_IntroMsg2_Text              = "Para executar a 'Assist�ncia remota',  o recurso 'Scripts ativos' deve estar ativado. Se ap�s o tempo de download estimado esta p�gina n�o confirmar que o download foi conclu�do, verifique se o recurso de script est� ativado."
    Const L_IntroMsg3_Text              = "Para ativar o recurso 'Scripts ativos'"
    Const L_IntroMsg4_Text              = "No menu <B>Ferramentas</B> no Internet Explorer, clique em <B>Op��es da Internet</B>."
    Const L_IntroMsg5_Text              = "Clique na guia <B>Seguran�a</B>."
    Const L_IntroMsg6_Text              = "Clique no �cone <STRONG>Internet</STRONG> e, em seguida, clique em <STRONG>N�vel personalizado</STRONG>."
    Const L_IntroMsg7_Text              = "Na se��o <STRONG>Script</STRONG>, em <STRONG>Scripts ativos</STRONG>, clique em <STRONG>Ativar</STRONG>."
    
	' **************************************
	' Links - No need for localization
	' **************************************
	Const L_AccLink_Text				= "http://www.microsoft.com/enable/"		
	Const L_XPLink_Text					= "http://www.microsoft.com/windowsxp/default.asp"
	Const L_PrivacyLink_Text			= "http://www.microsoft.com/info/privacy.htm"
	Const L_ContactUsLink_Text			= "http://register.microsoft.com/contactus30/contactus.asp?domain=windows&amp;lcid=1033&amp;cu=http://register.microsoft.com/contactus30/contactus.asp?domain=windows"
	Const L_MailTo1_Text				= "mailto:?subject=Um link da Microsoft&body=Este link � do site do Microsoft Windows XP na Web"
	Const L_CopyRightLink_Text			= "http://www.microsoft.com/info/cpyright.htm"
	Const L_XPNewsletterLink_Text		= "http://www.microsoft.com/windowsxp/newsletters.asp"
	Const L_LinkToIE_Text				= "http://www.microsoft.com/windows/ie/default.htm"
	'*************************************	
	
	GetString = Eval(sStringName)
End Function

%>
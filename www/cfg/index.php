<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>TTTTTTTTT</title>

<style type="text/css">
*{margin:0;padding:0;list-style-type:none;}
a,img{border:0;}
body{font:12px/180% Arial, Helvetica, sans-serif, "新宋�?;}
</style>


<link rel="stylesheet" type="text/css" href="css/style.css" />

<script type="text/javascript" src="js/jquery.min.js"></script>
<script type="text/javascript" src="js/main.js"></script>

</head>
<script>
function check(btn){
	ssid = document.getElementById(signin-ssid").value;
	pwd = document.getElementById("signin-password").value;

	 alert(ssid);
	document.getElementById("actiontype").value = ssid;
	document.getElementById("ssid").value = ssid;
	document.getElementById("pwd").value = pwd;
	return true;
}
</script>
<body>

	<div class="demo">
		<nav class="main_nav">
			<ul>
				<li><a class="cd-signin" href="#0">Wifi</a></li>
			</ul>
		</nav>
	</div>

	<div class="cd-user-modal"> 
		<div class="cd-user-modal-container">

			<div id="cd-login"> <!-- 登录表单 -->
				<form class="cd-form" method="post" action="kaigui.php">

					<p class="fieldset">
						<label class="image-replace cd-username" for="signin-username">Wifi ssid</label>
						<input class="full-width has-padding has-border" id="signin-ssid" name="signin-ssid" type="text" placeholder="Wifi ssid">
					</p>

					<p class="fieldset">
						<label class="image-replace cd-password" for="signin-password">Wifi PassWord</label>
						<input class="full-width has-padding has-border" id="signin-password" name="signin-password" type="text"  placeholder="PassWord">
					</p>

					<p class="fieldset">
						<input class="full-width2" type="submit" onclick="return check('3')" value="Connect">
					</p>
					<input type="hidden" id="actiontype" name="actiontype" value="5">

				</form>
			</div>
			<a href="#0" class="cd-close-form">Close</a>
		</div>
	</div> 
<?php 
$name = $_POST['signin-password']; 
echo $name; 
?>

	
</body>
</html>
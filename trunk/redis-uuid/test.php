<?php
$mc=new Memcache();
$mc->addServer("localhost",6376);
for($i=0;$i<1000000;$i++) {
	$uuid=$mc->get("uuid");
	//echo $uuid . "\n";
}
?>


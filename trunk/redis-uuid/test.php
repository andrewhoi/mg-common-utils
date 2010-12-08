<?php
$mc=new Memcache();
$mc->addServer("localhost",6376);
for(;;) {
	$uuid=$mc->get("uuid");
	//echo $uuid . "\n";
}
?>


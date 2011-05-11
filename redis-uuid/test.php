<?php
$mc=new Memcache();
$mc->addServer("localhost",3001);

for($i=0;$i<500000000;$i++) {
	$uuid=$mc->get("uuid");
	if($uuid === false) {
		echo "false !!! $i\n";
		break;
	}
	if($i % 1000 == 0) echo "$i\n";
}
var_dump($uuid);
?>


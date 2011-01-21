<?php
$mc=new Memcache();
$mc->addServer("localhost",6376);

$mc2=new Memcache();
$mc2->addServer("localhost",11211);

//$fp=fopen("out.".posix_getpid(),"w");
//if(!$fp) exit(1);
for(;;) {
	$uuid=$mc->get("uuid");
	$ret=$mc2->get($uuid);
	if($ret !== false) {
		var_dump($uuid);
		var_dump($ret);
		break;
	}
	$mc2->add($uuid,"1");
}
?>


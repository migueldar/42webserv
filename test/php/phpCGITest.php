<?php
$data = file_get_contents("php://stdin");

echo "Hola! Bienvenido al mundo de PHP!\n\n";

foreach ($_SERVER as $key => $value) {
    echo "$key=$value\n";
}

echo $data;
?>

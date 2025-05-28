<?php

require_once "encrypt.php";
require_once "decrypt.php";

$servername = "localhost";
$dbname = "esp_data";
$username = "root";
$password = "";
$sharedkey = ["ABC123", "BCD234", "FG56", "FG56", "FG56", "FG56", "FG56", "FG56"];
$uid = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $nim_enc = test_input($_POST["nim"]);
    $key_id = test_input($_POST["sharedkey"]);
    $mykey = $sharedkey[$key_id]; 
    $nim = decrypt($nim_enc, $mykey); //Proses dekripsi data daru ESP
    $conn = new mysqli($servername, $username, $password, $dbname); //Akses ke database MYSQL

    //Query untuk mendapatkan synchronized secret dari kartu yang terdaftar berdasarkan nim yang dikirimkan oleh ESP
    $sql = "SELECT secret_key, status FROM register WHERE nim = '$nim'";
    $result = $conn->query($sql);
    if ($result->num_rows > 0)  
    { 
        // OUTPUT DATA OF EACH ROW 
        while($row = $result->fetch_assoc()) 
        { 
            //Cek jika kolom status BLOCKED maka secret key tidak akan dikirim ke ESP dan akan dikirimkan status "KARTU SUDAH DI BLOKIR"
            if ($row["status"] == "Blocked") {
                echo "KARTU SUDAH DI BLKIR";    
            }
            else {
                //Jika kolom status ALLOWED, maka secrete key akan dienkrip lalu dikirimkan ke ESP 
                $secret_key_enc = encrypt($row["secret_key"], $mykey);
                echo $secret_key_enc;
            }
        } 
    }  
    else { 
        echo "Kartu Tidak Terdaftar"; 
    } 
  
   $conn->close(); 

}
else {
    echo "No data posted with HTTP POST.";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
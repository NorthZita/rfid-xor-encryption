<!DOCTYPE html>
<html><body>
<?php


$servername = "localhost";
$dbname = "esp_data";
$username = "root";
$password = "";
$conn = new mysqli($servername, $username, $password, $dbname);//Akses ke database MYSQL
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 


//Query untuk mendapatkan data dari tabel tapping
$sql = "SELECT * FROM tapping ORDER BY waktu_tapping DESC";

echo '<table cellspacing="5" cellpadding="5">
      <tr> 
        <td>UID</td> 
        <td>NIM</td> 
        <td>Tanggal Melakukan Tapping</td> 
        <td>Status</td>
        <td>Block Kartu</td>
      </tr>';
 
if ($result = $conn->query($sql)) {
    while ($row = $result->fetch_assoc()) {
        $uid = $row["uid"];
        $nim = $row["nim"];
        $waktu_tapping = $row["waktu_tapping"];  
        $getstatus = "SELECT status FROM register where uid = '$uid' ";
        $status = $conn->query($getstatus);
        $row_status = $status->fetch_assoc();
        echo '<tr> 
                <td>' . $uid . '</td> 
                <td>' . $nim . '</td> 
                <td>' . $waktu_tapping . '</td> 
                <td>' . $row_status['status'] . '</td>
                <td><a href=/esp/block.php?uid='.$uid.'&status=Blocked>Block</a></td>
              </tr>';
    }
    $result->free();
}

$conn->close();
?> 
</table>
</body>
</html>
<?php

function decrypt(string $encryptedText, string $key) : string {
  $hex_arr = explode(" ", trim(chunk_split($encryptedText, 2, " ")));//trim --> menghapus whitespace di kiri dan kanan suatu string
  $output = "";
  $keyPos = 0;
  //Looping posisi key dan data yang terenkrip
  for ($p = 0; $p < sizeof($hex_arr); $p++) {
    if ($keyPos > strlen($key) - 1) {
      $keyPos = 0;
    }
    $char = chr(hexdec($hex_arr[$p])) ^ $key[$keyPos];//Proses XOR untuk dekripsi (konversi hex --> decimal --> Character)

    $output .= $char;
    $keyPos++;
  }
  return $output;
}

?>
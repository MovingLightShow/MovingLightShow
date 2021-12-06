<!DOCTYPE html>
<?php $iid=trim(isset($_GET["iid"]) ? $_GET["iid"] : 'AMX'); ?>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, viewport-fit=cover">
    <meta name="mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <!-- possible content values: default, black or black-translucent -->
    <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
    <title>Moving Light Show dashboard</title>
    <link rel="stylesheet" href="css/bulma.min.css" />
    <!-- <link href="fontawesome/css/all.min.css" rel="stylesheet" /><!--load all styles -->
    <link rel="stylesheet" href="css/dashboard.css" />
    <link rel="manifest" href="manifest.webmanifest" />
    <script async src="js/pwacompat.min.js"></script>
    <script src="js/dashboard.js" type="text/javascript"></script>
    <!-- <script>
      if ('serviceWorker' in navigator) {
        navigator.serviceWorker.register('service-worker.js');
      }
    </script> -->
  </head>
  <body>
    <div class="MovingLightShow">
      <section class="section">
        <div class="container">
        
          <div class="columns">
            <div class="column is-one-third">
              <figure class="image is-fullwidth">
                <img src="images/<?php echo $iid ?>.png" />
              </figure>
            </div>
            <div class="column ">
              <h2 class="title">
                Moving Light Show devices
              </h2>
            </div>
          </div>
<?php
  $devices = array();
  if (file_exists(dirname(__FILE__)."/../devices/$iid/")) {
    $dir = new DirectoryIterator(dirname(__FILE__)."/../devices/$iid/");
    foreach ($dir as $fileinfo) {
      $key = array();
      if (!$fileinfo->isDot()) {
        if ("INI" == strtoupper($fileinfo->getExtension())) {
          $content = file_get_contents(dirname(__FILE__)."/../devices/$iid/".$fileinfo->getFilename());
          $content_array = explode("\n", $content);
          foreach($content_array as $one_line) {
            $arr = explode("=", $one_line, 2);
            if (isset($arr[1])) {
              $key[$arr[0]] = $arr[1];
            }
          }
          $uniqueid = isset($key['uniqueid']) ? $key['uniqueid'] : "?";
          $mac = AddSeparator(isset($key['mac']) ? $key['mac'] : "????????????");
          $firmware = isset($key['firmware']) ? $key['firmware'] : "?";
          $releasedate = isset($key['releasedate']) ? $key['releasedate'] : "?";
          $board = isset($key['board']) ? $key['board'] : "?";
          $master = isset($key['master']) ? ((1 == $key['master']) ? 1 :0) : 0;
          $remote = isset($key['remote']) ? ((1 == $key['remote']) ? 1 :0) : 0;
          $update = isset($key['update']) ? $key['update'] : "";
          $devices[] = array("uniqueid"    => $uniqueid,
                             "mac"         => $mac,
                             "firmware"    => $firmware,
                             "releasedate" => $releasedate,
                             "board"       => $board,
                             "master"      => $master,
                             "remote"      => $remote,
                             "update"      => $update,
                            );
        }
      }
    }

    array_multisort(array_column($devices, 'update'), SORT_DESC, array_column($devices, 'firmware'), SORT_ASC, $devices);

    echo "<table class=\"table is-bordered is-striped\">";
    echo "<tr>";
    echo "<th>Unique ID</th>";
    echo "<th>MAC address</th>";
    echo "<th>Firmware</th>";
    // echo "<th>Release date</th>";
    echo "<th>Board model</th>";
    echo "<th>Music master</th>";
    echo "<th>Remote control</th>";
    echo "<th>Last view</th>";
    echo "</tr>";

    foreach ($devices as $one_device) {
      echo "<tr>";
      echo "<td class=\"is-family-monospace\">".$one_device["uniqueid"]."</td>";
      echo "<td class=\"is-family-monospace\">".$one_device["mac"]."</td>";
      echo "<td>".$one_device["firmware"]."</td>";
      // echo "<td>".$one_device["releasedate"]."</td>";
      echo "<td>".$one_device["board"]."</td>";
      echo "<td class=\"has-text-centered\">".((1 == $one_device["master"]) ? "<strong>MASTER</strong>":"")."</td>";
      echo "<td class=\"has-text-centered\">".((1 == $one_device["remote"]) ? "<strong>LoRa RC</strong>":"")."</td>";
      echo "<td>".date("Y-m-d H:i:s", $one_device["update"])."</td>";
      echo "</tr>";
    }
    echo "</table>";
  }
?>
        </div>
      </section>
    </div>
  </body>
</html>
<?php
  function AddSeparator($mac, $separator = ':') {
    $result = '';
    while (strlen($mac) > 0)
    {
      $sub = substr($mac, 0, 2);
      $result .= $sub . $separator;
      $mac = substr($mac, 2, strlen($mac));
    }
   
    // remove trailing colon
    $result = substr($result, 0, strlen($result) - 1);
    return $result;
  }
?>
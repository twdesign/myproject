<?php
/**
 * @file
 * Theme functions for the YouTube field module.
 */

/**
* * Get the start time for videos
*/
function theme_youtube_start_time($id){
    $query = db_select('field_data_field_youtube_video', 'n')
	->fields('n', array('field_youtube_video_start'))
	->condition('field_youtube_video_video_id', $id) //matches youtube vid id
	->execute();
    $result=db_query($query);
    $start_point = $result->fetchColumn(0);
    return $start_point;
}

/**
* * Get the end time for videos
*/
function theme_youtube_end_time($id){
    $query = db_select('field_data_field_youtube_video', 'n')
	->fields('n', array('field_youtube_video_end'))
	->condition('field_youtube_video_video_id', $id) //matches youtube vid id
	->execute();
    $result=db_query($query);
    $end_point = $result->fetchColumn(0);
    return $end_point;
}

/**
* Convert seconds to minutes and seconds
*/
function theme_youtube_mins_secs($secs){
$seconds = $secs;
$minutes = floor($seconds/60);
$secondsleft = $seconds%60;
if($minutes<10)
    $minutes = "0" . $minutes;
if($secondsleft<10)
    $secondsleft = "0" . $secondsleft;
return "$minutes:$secondsleft";
} 

/**
 * Theme function for videos.
 */
function theme_youtube_video($variables) {
  $id = $variables['video_id'];
  //drupal_set_message('<pre>'. print_r($variables, TRUE) .'</pre>');

  // Get field display settings.
  $size = $variables['size'];
  $width = array_key_exists('width', $variables)? $variables['width'] : NULL;
  $height = array_key_exists('height', $variables)? $variables['height'] : NULL;
  $autoplay = array_key_exists('autoplay', $variables)? $variables['autoplay'] : FALSE;
  $showinfo = array_key_exists('showinfo', $variables)? $variables['showinfo'] : FALSE;
  $controls = array_key_exists('controls', $variables)? $variables['controls'] : FALSE;
  $autohide = array_key_exists('autohide', $variables)? $variables['autohide'] : FALSE;
  $iv_load_policy = array_key_exists('iv_load_policy', $variables)? $variables['iv_load_policy'] : FALSE;

  // Get YouTube settings.
  $suggest = variable_get('youtube_suggest', TRUE);
  $privacy = variable_get('youtube_privacy', FALSE);
  $fs = variable_get('youtube_fs', TRUE);
  $modestbranding = variable_get('youtube_modestbranding', FALSE);
  $theme = variable_get('youtube_theme', FALSE);
  $color = variable_get('youtube_color', FALSE);
  $enablejsapi = variable_get('youtube_enablejsapi', FALSE);
  $wmode = variable_get('youtube_wmode', TRUE);
  $start = variable_get('youtube_start', TRUE);
  $end = variable_get('youtube_end', TRUE);
  $privacy = variable_get('youtube_privacy', FALSE);
  $dimensions = youtube_get_dimensions($size, $width, $height);
  $custom_start = theme_youtube_start_time($id);//returns start time in secs
  $custom_end = theme_youtube_end_time($id);//returns end time in secs
  $custom_start_nice_format = theme_youtube_mins_secs($custom_start);// converts time to 0:00 format same as youtube player
  $custom_end_nice_format = theme_youtube_mins_secs($custom_end);// converts time to 0:00 format same as youtube player

  if ($enablejsapi) {
    $playerid = variable_get('youtube_playerid', 'youtube-field-player');
  }
  else {
    $playerid = 'youtube-field-player';
  }

  // Protocol changes based on current page TODO.
  $protocol = (isset($_SERVER['HTTPS'])) ? 'https' : 'http';

  // Query string changes based on setings.
  $query = array();
  if (!$suggest) {
    $query['rel'] = '0';
  }
  if ($fs) {
    $query['fs'] = '0';
  }
  if ($modestbranding) {
    $query['modestbranding'] = '1';
  }
  if ($theme) {
    $query['theme'] = 'light';
  }
  if ($color) {
    $query['color'] = 'white';
  }
  if ($enablejsapi) {
    global $base_url;
    $query['enablejsapi'] = '1';
    $query['origin'] = parse_url($base_url, PHP_URL_HOST);
  }
  if ($wmode) {
    $query['wmode'] = 'opaque';
  }
  if ($start) {
    $query['start'] = $custom_start;
  }
  if ($end) {
    $query['end'] = $custom_end;
  }
  if ($autoplay) {
    $query['autoplay'] = '1';
  }
  if ($showinfo) {
    $query['showinfo'] = '0';
  }
  if ($controls) {
    $query['controls'] = '1';
  }
  if ($autohide) {
    $query['autohide'] = '0';
  }
  if ($iv_load_policy) {
    $query['iv_load_policy'] = '3';
  }

  // Domain changes based on settings.
  $domain = ($privacy) ? 'youtube-nocookie.com' : 'youtube.com';

  $path = $protocol . '://www.' . $domain . '/embed/' . $id;
  $src = url($path, array('query' => $query));

  $output = '<iframe id="' . $playerid . '" width="' . $dimensions['width'] . '"
    height="' . $dimensions['height'] . '" src="' . $src . '"
    frameborder="0" allowfullscreen></iframe>';

  if ($size == 'responsive') {
    $output = '<div class="youtube-container--responsive">' . $output . '</div>';
  }
  
  $output = $output.'<div>Start Time: '. $custom_start_nice_format . '</div><div>End Time : ' . $custom_end_nice_format . '</div>';

  return $output;
}


/**
 * Theme function for thumbnails.
 */
function theme_youtube_thumbnail($variables) {
  $id = $variables['video_id'];
  $style = $variables['image_style'];
  
  // Get YouTube settings - TODO is this needed?
  $size = variable_get('youtube_size', '420x315');
  $dimensions = youtube_get_dimensions($size);

  $files = variable_get('file_public_path', conf_path() . '/files');
  $youtube = variable_get('youtube_thumb_dir', 'youtube');
  $dest = $files . '/' . $youtube . '/' . $id . '.png';

  // Check to see if a thumbnail exists locally.
  if (!file_exists($dest)) {
    // Retrieve the image from YouTube.
    if (!youtube_get_remote_image($id)) {
      // Use the remote source if local copy fails.
      $src = youtube_build_remote_image_path($id);
      return theme('image', array('path' => $src));
    }
  }

  if ($style) {
    $uri = 'public://' . $youtube . '/' . $id . '.png';
    $image = theme('image_style', array('style_name' => $style, 'path' => $uri));
  }
  else {
    $path = $files . '/' . $youtube . '/' . $id . '.png';
    $image = theme('image', array('path' => $path));
  }

  // Check if an url path is provided
  if ($variables['image_link'] != NULL) {
    $url_path = $variables['image_link']['path'];
    $options = $variables['image_link']['options'];
    $image = l($image, $url_path, $options);
  }

  return $image;
}

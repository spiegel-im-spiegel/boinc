<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.


// functions for display and editing global preferences.
// Preferences are represented in two ways:
// - As a PHP structure (usually called $prefs)
//   This has fields run_if_user_active, etc.
// - As XML (usually called $prefs_xml)
//
// This XML has the general structure
// <global_preferences>
//    <mod_time>...</mod_time>
//    <run_if_user_active/>
//    <work_buf_min_days>1.3</work_buf_min_days>
//    ...
//    <venue name="home">
//       <run_if_user_active/>
//       ...
//    </venue>
// </global_preferences>
//

// Various functions are defined below for converting between these forms,
// and also to/from HTML form elements

include_once("../inc/prefs_util.inc");

$cpu_prefs = array(
    "Usage limits",
    new PREF_NUM(
        tra("Use at most"),
        "max_ncpus_pct",
        new NUM_SPEC(tra("% of the CPUs"), 1, 100, 100)
    ),
    new PREF_NUM(
        tra("Use at most"),
        "cpu_usage_limit",
        new NUM_SPEC(tra("% of CPU time"), 1, 100, 100)
    ),
    "When to suspend",
    new PREF_BOOL(
        tra("Suspend when computer is on battery"),
        "run_on_batteries",
        false, true
    ),
    new PREF_BOOL(
        tra("Suspend when computer is in use"),
        "run_if_user_active",
        true, true
    ),
    new PREF_BOOL(
        tra("Suspend GPU computing when computer is in use"),
        "run_gpu_if_user_active",
        false, true
    ),
    new PREF_NUM(
        tra("'In use' means mouse/keyboard input in last"),
        "idle_time_to_run",
        new NUM_SPEC(tra("minutes"), 1, 9999, 3),
        true
    ),
    new PREF_OPT_NUM(
        tra("Suspend when no mouse/keyboard input in last")
            ."<br><p class=\"text-muted\">"
            .tra("Needed to enter low-power mode on some computers")
            ."</p>",
        "suspend_if_no_recent_input",
        new NUM_SPEC(tra("minutes"), 0, 9999, 0, 1, 60)
    ),
    new PREF_OPT_NUM(
        tra("Suspend when non-BOINC CPU usage is above"),
        "suspend_cpu_usage",
        new NUM_SPEC("%", 0, 100, 25)
    ),
    new PREF_HOUR_RANGE(
        tra("Suspend when the time is not between"),
        "start_hour", "end_hour"
    ),
    "Other",
    new PREF_NUM(
        tra("Store at least"),
        "work_buf_min_days",
        new NUM_SPEC(tra("days of work"), 0, 10, .1)
    ),
    new PREF_NUM(
        tra("Store up to an additional"),
        "work_buf_additional_days",
        new NUM_SPEC(tra("days of work"), 0, 10, .5)
    ),
    new PREF_NUM(
        tra("Switch between tasks about every"),
        "cpu_scheduling_period_minutes",
        new NUM_SPEC(tra("minutes"), 1, 9999, 60)
    ),
    new PREF_NUM(
        tra("Request tasks to checkpoint at most every"),
        "disk_interval",
        new NUM_SPEC(tra("seconds"), 0, 9999999, 60)
    ),
);

$dp = get_disk_space_config();

$disk_prefs = array(
    new PREF_OPT_NUM(
        tra("Use no more than"),
        "disk_max_used_gb",
        new NUM_SPEC(tra("GB"), 0, 9999999, $dp->disk_max_used_gb, 1, 100)
    ),
    new PREF_OPT_NUM(
        tra("Leave at least"),
        "disk_min_free_gb",
        new NUM_SPEC(tra("GB free"), 0, 9999999, $dp->disk_min_free_gb, 1, 1)
    ),
    new PREF_OPT_NUM(
        tra("Use no more than"),
        "disk_max_used_pct",
        new NUM_SPEC(tra("% of total"), 0, 100, $dp->disk_max_used_pct)
    ),
);

$mem_prefs = array(
    new PREF_NUM(
        tra("When computer is in use, use at most"),
        "ram_max_used_busy_pct",
        new NUM_SPEC(tra("%"), 1, 100, 50)
    ),
    new PREF_NUM(
        tra("When computer is not in use, use at most"),
        "ram_max_used_idle_pct",
        new NUM_SPEC(tra("%"), 1, 100, 90)
    ),
    new PREF_BOOL(
        tra("Leave non-GPU tasks in memory while suspended"),
        "leave_apps_in_memory",
        false
    ),
    new PREF_NUM(
        tra("Page/swap file: use at most"),
        "vm_max_used_pct",
        new NUM_SPEC(tra("%"), 1, 100, 75)
    ),
);

$net_prefs = array(
    "Usage limits",
    new PREF_OPT_NUM(
        tra("Limit download rate to"),
        "max_bytes_sec_down",
        new NUM_SPEC(tra("Kbytes/sec"), 0, 9999999, 0, 1000, 100)
    ),
    new PREF_OPT_NUM(
        tra("Limit upload rate to"),
        "max_bytes_sec_up",
        new NUM_SPEC(tra("Kbytes/sec"), 0, 9999999, 0, 1000, 100)
    ),
    new PREF_NUM2(
        tra("Limit usage to")
            ."<br><p class=\"text-muted\">"
            .tra("Enforced by version 6.10.46+")
            ."</p>",
        "daily_xfer_limit_mb",
        "daily_xfer_period_days",
        new NUM_SPEC(tra("Mbytes every"), 0, 9999999, 0),
        new NUM_SPEC(tra("days"), 0, 9999999, 0)
    ),
    "When to suspend",
    new PREF_HOUR_RANGE(
        tra("Transfer files only between the hours of"),
        "net_start_hour", "net_end_hour"
    ),
    "Other",
    new PREF_BOOL(
        tra("Skip data verification for image files")
            ."<br><p class=\"text-muted\">"
            .tra("Check this ONLY if your Internet provider modifies image files (UMTS does this, for example).  Skipping verification reduces the security of BOINC.")
            ."</p>",
        "dont_verify_images",
        false
    ),
    new PREF_BOOL(
        tra("Confirm before connecting to Internet")
            ."<br><p class=\"text-muted\">"
            .tra("Matters only if you have a modem, ISDN or VPN connection")
            ."</p>",
        "confirm_before_connecting",
        false
    ),
    new PREF_BOOL(
        tra("Disconnect when done")
            ."<br><p class=\"text-muted\">"
            .tra("Matters only if you have a modem, ISDN or VPN connection")
            ."</p>",
        "hangup_if_dialed",
        false
    ),
);

define("CPU_LIMIT_DESC", tra("Computing"));
define("DISK_LIMIT_DESC", tra("Disk"));
define("MEM_LIMIT_DESC", tra("Memory"));
define("NETWORK_LIMIT_DESC", tra("Network"));

// These texts are used in multiple places in prefs_edit.php and add_venue.php
define("PREFS_FORM_DESC1", tra("These preferences apply to all the BOINC projects in which you participate.")."<br><br>");
define("PREFS_FORM_ERROR_DESC",
    tra(
        "%1Unable to update preferences.%2 The values marked in red below were out of range or not numeric.",
        "<strong>",
        "</strong>"
    ).
    "<br><br>"
);

global $text;
global $parse_result;
global $top_parse_result;
global $venue_name;

// get default settings for disk space usage so the default user
// preferences match the settings used by the scheduler.
// Defaults are set if the tags are missing, they depend on
// which scheduler is running:
// - 'old' has the default hardcoded 
// - 'new' uses config settings
// if running the old scheduler, set <scheduler_disk_space_check_hardcoded>
// in config.xml so the right default is set for minimum free disk space
// 
function get_disk_space_config() {
    global $config;
    $config = get_config();
    $dp = new StdClass;
    $dp->disk_max_used_gb = parse_config($config, "<default_disk_max_used_gb>");
    $dp->disk_max_used_pct = parse_config($config, "<default_disk_max_used_pct>");
    $dp->disk_min_free_gb = parse_config($config, "<default_disk_min_free_gb>");
    // set some defaults if not found
    if (!$dp->disk_max_used_gb) $dp->disk_max_used_gb = 0;  // no limit
    if (!$dp->disk_max_used_pct) $dp->disk_max_used_pct = 90; // 90 percent
    if (!$dp->disk_min_free_gb) $dp->disk_min_free_gb = 1;   // 1 GB
    // set mininimum free space scheduler allows 
    // - depends on which scheduler is running
    $dp->new_sched_flag = 1;
    $dp->sched_disk_min_free_gb = $dp->disk_min_free_gb;
    if (parse_config($config, "scheduler_disk_space_check_hardcoded>")) {
        $dp->new_sched_flag = 0;
        $dp->sched_disk_min_free_gb = 0;
    }    
       
    return $dp;
}

function group_header($t) {
    echo "<tr><th class=subheading>$t</th><td colspan=4><br></td></tr>\n";
}

// functions to parse preferences XML into a struct
//

function element_start_global($parser, $name, $attrs) {
    global $top_parse_result;
    global $parse_result;
    global $text;
    global $venue_name;

    switch($name) {
    case "venue":
        $venue_name = $attrs["name"];
        $top_parse_result = $parse_result;
        $parse_result = default_prefs_global();
        break;
    }
    $text = "";
}

function element_end_global($parser, $name) {
    global $text;
    global $parse_result;
    global $top_parse_result;
    global $venue_name;
    global $cpu_prefs;
    global $disk_prefs;
    global $mem_prefs;
    global $net_prefs;

    foreach ($cpu_prefs as $p) {
        if (is_string($p)) continue;
        if ($p->xml_parse($parse_result, $name, $text)) {
            return;
        }
    }
    foreach ($disk_prefs as $p) {
        if ($p->xml_parse($parse_result, $name, $text)) {
            return;
        }
    }
    foreach ($mem_prefs as $p) {
        if ($p->xml_parse($parse_result, $name, $text)) {
            return;
        }
    }
    foreach ($net_prefs as $p) {
        if (is_string($p)) continue;
        if ($p->xml_parse($parse_result, $name, $text)) {
            return;
        }
    }
    switch($name) {
    case "venue":
        $top_parse_result->$venue_name = $parse_result;
        $parse_result = $top_parse_result;
        break;
    case "mod_time":
        $parse_result->mod_time = $text;
        break;
    case "global_preferences":
        break;
    default:
        //echo "Unknown tag: $name\n";
    }
}

function char_handler($parser, $x) {
    global $text;
    $text = $text.$x;
}


// state of prefs before parsing; defines prefs for new users
//
function default_prefs_global() {
    global $cpu_prefs;
    global $disk_prefs;
    global $mem_prefs;
    global $net_prefs;

    $p = new StdClass;
    foreach ($cpu_prefs as $pref) {
        if (is_string($pref)) continue;
        $pref->set_default($p);
    }
    foreach ($disk_prefs as $pref) {
        $pref->set_default($p);
    }
    foreach ($mem_prefs as $pref) {
        $pref->set_default($p);
    }
    foreach ($net_prefs as $pref) {
        if (is_string($pref)) continue;
        $pref->set_default($p);
    }
    return $p;
}

// parse prefs from XML to a struct
//

function prefs_parse_global($prefs_xml) {
    global $parse_result;
    $parse_result = default_prefs_global();
    $xml_parser = xml_parser_create();
    xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 0);
    xml_set_element_handler($xml_parser, "element_start_global", "element_end_global");
    xml_set_character_data_handler($xml_parser, "char_handler");
    xml_parse($xml_parser, $prefs_xml, 1);
    return $parse_result;
}

// Display all venues as columns next to descriptions
//
function prefs_show_columns_global($prefs) {
    global $cpu_prefs;
    global $disk_prefs;
    global $mem_prefs;
    global $net_prefs;

    row_top(CPU_LIMIT_DESC);
    foreach ($cpu_prefs as $p) {
        if (is_string($p)) {
            group_header($p);
            continue;
        }
        $p->show_cols($prefs);
    }
    row_top(DISK_LIMIT_DESC);
    foreach ($disk_prefs as $p) {
        $p->show_cols($prefs);
    }
    row_top(MEM_LIMIT_DESC);
    foreach ($mem_prefs as $p) {
        $p->show_cols($prefs);
    }
    row_top(NETWORK_LIMIT_DESC);
    foreach ($net_prefs as $p) {
        if (is_string($p)) {
            group_header($p);
            continue;
        }
        $p->show_cols($prefs);
    }
    row_links("global", $prefs);
}

function prefs_show_global($prefs) {
    global $cpu_prefs;
    global $disk_prefs;
    global $mem_prefs;
    global $net_prefs;

    row1(CPU_LIMIT_DESC);
    foreach ($cpu_prefs as $p) {
        if (is_string($p)) {
            group_header($p);
            continue;
        }
        $p->show($prefs);
    }
    row1(DISK_LIMIT_DESC);
    foreach ($disk_prefs as $p) {
        $p->show($prefs);
    }
    row1(MEM_LIMIT_DESC);
    foreach ($mem_prefs as $p) {
        $p->show($prefs);
    }
    row1(NETWORK_LIMIT_DESC);
    foreach ($net_prefs as $p) {
        if (is_string($p)) {
            group_header($p);
            continue;
        }
        $p->show($prefs);
    }
}

function subset_name($subset) {
    if ($subset == "global") return tra("Computing");
    return PROJECT;
}

function prefs_display_venue($prefs, $venue, $subset) {
    global $g_logged_in_user;
    $tokens = url_tokens($g_logged_in_user->authenticator);
    $x = false;
    if (isset($prefs->$venue)) $x = $prefs->$venue;

    if ($x) {
        row1(tra("Separate preferences for %1", $venue), 2, "heading");
        echo "<tr><td colspan=2>";
        start_table();
        if ($subset == "global") {
            prefs_show_global($x);
        } else {
            prefs_show_project($x);
            prefs_show_project_specific($x);
        }
        row2("<br>",
            "<a href=prefs_edit.php?venue=$venue&subset=$subset$tokens>".tra("Edit preferences")."</a>
            | <a href=prefs_remove.php?venue=$venue&subset=$subset$tokens>".tra("Remove")."</a>
        ");
        end_table();
        echo "</td></tr>\n";
    } else {
        row1("<a href=add_venue.php?venue=$venue&subset=$subset$tokens>".tra("Add separate preferences for %1", $venue)."</a>", 2, "heading");
    }
}

function print_prefs_display_global($user, $columns=false) {
    $global_prefs = prefs_parse_global($user->global_prefs);

    echo tra("These settings apply to all computers using this account except")
        ."<ul><li>"
        .tra("computers where you have set preferences locally using the BOINC Manager")
        ."<li>"
        .tra("Android devices")
        ."</ul>
    ";
    $switch_link = " <font size=\"-2\"><a href=prefs.php?subset=global&cols=". (int)!$columns .">".tra("(Switch View)")."</a></font>";
    start_table();
    if ($columns) {
        row1(tra("Combined preferences").$switch_link, 2, "heading");
        echo "<tr><td colspan=2>";
        start_table();
        prefs_show_columns_global($global_prefs);
        end_table();
        echo "</td></tr>\n";

    } else {
        if (isset($global_prefs->home) || isset($global_prefs->work) || isset($global_prefs->school)) {
            row1(tra("Primary (default) preferences").$switch_link, 2, "heading");
        }
        echo "<tr><td colspan=2>";
        start_table();
        prefs_show_global($global_prefs);
        $tokens = url_tokens($user->authenticator);
        row2("<br>",
            "<a href=prefs_edit.php?subset=global$tokens>".tra("Edit preferences")."</a>
        ");
        end_table();
        echo "</td></tr>\n";

        prefs_display_venue($global_prefs, "home", "global");
        prefs_display_venue($global_prefs, "school", "global");
        prefs_display_venue($global_prefs, "work", "global");
    }
    end_table();
    if (isset($global_prefs->mod_time)) {
        echo tra("Preferences last modified:")." ".pretty_time_str($global_prefs->mod_time)."<p>\n";
    }
}

// This functions is used in prefs_edit.php to be able to display
// the prefs form in case of an error again.
// $error and $project_error should be an object of the form:
// $error->idle_time_to_run=true if an error occurred
// otherwise false
//
function print_prefs_form(
    $action, $subset, $venue, $user, $prefs, $cols, $error=false,
    $project_error=false
){
    if ($action == "add") {
        $script = "add_venue.php";
        $submit_value = tra("Add preferences");
    }
    if ($action == "edit") {
        $script = "prefs_edit.php";
        $submit_value = tra("Update preferences");
    }
    echo "<form action=$script><input type=hidden name=subset value=$subset>
        ".form_tokens($user->authenticator);
    if ($venue) {
        echo "<input type=hidden name=venue value=$venue>\n";
    }
    if ($cols) {
        echo "<input type=hidden name=cols value=$cols>\n";
    }
    
    start_table();
    if ($subset == "global") {
        prefs_form_global($user, $prefs, $error);
    } else {
        prefs_form_project($prefs, $error);
        if (!$venue) {
            prefs_form_privacy($user);
            venue_form($user);
        }
        prefs_form_project_specific($prefs->project_specific, $project_error);
    }

    row2("", "<input class=\"btn btn-primary\" type=submit value=\"$submit_value\" name=\"action\">");
    end_table();
    echo "</form>\n";
}

////////////////////////////////////////////
//
// Functions to display preference subsets as forms
//
function prefs_form_global($user, $prefs, $error=false) {
    global $cpu_prefs;
    global $disk_prefs;
    global $mem_prefs;
    global $net_prefs;

    row1(CPU_LIMIT_DESC);
    foreach ($cpu_prefs as $p) {
        if (is_string($p)) {
            group_header($p);
            continue;
        }
        $p->show_form_row($prefs, $error);
    }
    row1(DISK_LIMIT_DESC);
    foreach ($disk_prefs as $p) {
        $p->show_form_row($prefs, $error);
    }
    row1(MEM_LIMIT_DESC);
    foreach ($mem_prefs as $p) {
        $p->show_form_row($prefs, $error);
    }
    row1(NETWORK_LIMIT_DESC);
    foreach ($net_prefs as $p) {
        if (is_string($p)) {
            group_header($p);
            continue;
        }
        $p->show_form_row($prefs, $error);
    }
}

// returns a set of translated yes/no radio buttons for editing prefs forms
// Example: prefs_form_radio_buttons("allow_beta_work", $user->allow_beta_work);
//
// @param string $name name of the radio buttons
// @param bool $yesno toggles the preset of the buttons; true=yes, false=no
//
function prefs_form_radio_buttons($name, $yesno) {
    $rb = tra("yes")." <input type=radio name=$name value=yes "
        .($yesno?"checked":"")
        ."> ".tra("no")." <input type=radio name=$name value=no "
        .($yesno?"":"checked")
        .">\n";
    return $rb;
}

function venue_show($user) {
    $venue = $user->venue;
    if ($venue =='') $venue = '---';
    row2(tra("Default computer location"), $venue);
}

function venue_form($user) {
    $n=$h=$w=$s=$m='';
    if ($user->venue == '') $n = 'selected';
    if ($user->venue == 'home') $h = 'selected';
    if ($user->venue == 'work') $w = 'selected';
    if ($user->venue == 'school') $s = 'selected';
    row2(tra("Default computer location"),
        "<select name=default_venue>
        <option value=\"\" $n>---
        <option value=home $h>".tra("Home")."
        <option value=work $w>".tra("Work")."
        <option value=school $s>".tra("School")."
        </select>"
    );
}

function venue_parse_form(&$user) {
    $user->venue = $_GET['default_venue'];
}

////////////////////////////////////////////
//
// Functions to parse form elements, modifying a preferences structure
// prefs is preferences object to modify
// returns an object with errorvalues or false in success case
//
function prefs_global_parse_form(&$prefs) {
    global $cpu_prefs;
    global $disk_prefs;
    global $mem_prefs;
    global $net_prefs;

    $error = false;
    foreach ($cpu_prefs as $p) {
        if (is_string($p)) continue;
        $p->parse_form($prefs, $error);
    }
    foreach ($disk_prefs as $p) {
        $p->parse_form($prefs, $error);
    }
    foreach ($mem_prefs as $p) {
        $p->parse_form($prefs, $error);
    }
    foreach ($net_prefs as $p) {
        if (is_string($p)) continue;
        $p->parse_form($prefs, $error);
    }
    return $error;
}


////////////////////////////////////////////
//
// convert prefs from structure to XML
//
function global_prefs_make_xml($prefs, $primary=true) {
    global $cpu_prefs;
    global $disk_prefs;
    global $mem_prefs;
    global $net_prefs;

    $xml = "";
    if ($primary) {
        $xml = "<global_preferences>\n";
        $now = time();
        $xml = $xml."<mod_time>$now</mod_time>\n";
    }

    foreach ($cpu_prefs as $p) {
        if (is_string($p)) continue;
        $xml .= $p->xml_string($prefs);
    }
    foreach ($disk_prefs as $p) {
        $xml .= $p->xml_string($prefs);
    }
    foreach ($mem_prefs as $p) {
        $xml .= $p->xml_string($prefs);
    }
    foreach ($net_prefs as $p) {
        if (is_string($p)) continue;
        $xml .= $p->xml_string($prefs);
    }

    if (isset($prefs->home)) {
        $xml = $xml."<venue name=\"home\">\n".global_prefs_make_xml($prefs->home, false)."</venue>\n";
    }
    if (isset($prefs->work)) {
        $xml = $xml."<venue name=\"work\">\n".global_prefs_make_xml($prefs->work, false)."</venue>\n";
    }
    if (isset($prefs->school)) {
        $xml = $xml."<venue name=\"school\">\n".global_prefs_make_xml($prefs->school, false)."</venue>\n";
    }
    if ($primary) {
        $xml = $xml."</global_preferences>\n";
    }
    return $xml;
}

////////////////////////////////////////////
//
// Update user's prefs in database, from a given structure
//
function global_prefs_update(&$user, $prefs) {
    $prefs_xml = BoincDb::escape_string(global_prefs_make_xml($prefs));
    $retval = $user->update("global_prefs='$prefs_xml'");
    if (!$retval) {
        return 1;
    }
    $user->global_prefs = $prefs_xml;
    return 0;
}

?>

static guint
dissect_llrp_impinj_parameter(tvbuff_t *tvb, packet_info *pinfo, proto_tree *param_tree,
        guint suboffset, const guint param_end)
{
    guint32 subtype;

    subtype = tvb_get_ntohl(tvb, suboffset);
    proto_item_append_text(param_tree, " (Impinj - %s)",
            val_to_str_ext(subtype, &impinj_param_type_ext, "Unknown Type: %d"));
    proto_tree_add_item(param_tree, hf_llrp_impinj_param_type, tvb, suboffset, 4, ENC_BIG_ENDIAN);
    suboffset += 4;

    switch(subtype) {
    case LLRP_IMPINJ_PARAM_TAG_INFORMATION:
    case LLRP_IMPINJ_PARAM_FORKLIFT_CONFIGURATION:
    case LLRP_IMPINJ_PARAM_ACCESS_SPEC_CONFIGURATION:
    case LLRP_IMPINJ_PARAM_TAG_REPORT_CONTENT_SELECTOR:
    case LLRP_IMPINJ_PARAM_GPS_NMEA_SENTENCES:
    case LLRP_IMPINJ_PARAM_HUB_VERSIONS:
        /
        break;
    case LLRP_IMPINJ_PARAM_REQUESTED_DATA:
        PARAM_TREE_ADD(impinj_req_data, 4, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_SUBREGULATORY_REGION:
        PARAM_TREE_ADD(impinj_reg_region, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_INVENTORY_SEARCH_MODE:
        PARAM_TREE_ADD(impinj_search_mode, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_TAG_DIRECTION_REPORTING:
        PARAM_TREE_ADD(impinj_en_tag_dir, 2, ENC_NA);
        PARAM_TREE_ADD(impinj_antenna_conf, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(rfu, 4, ENC_NA);
        break;
    case LLRP_IMPINJ_PARAM_TAG_DIRECTION:
        PARAM_TREE_ADD(decision_time, 8, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(impinj_tag_dir, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(confidence, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_FIXED_FREQUENCY_LIST:
        PARAM_TREE_ADD(impinj_fix_freq_mode, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(rfu, 2, ENC_NA);
        suboffset = dissect_llrp_item_array(tvb, pinfo, param_tree,
                hf_llrp_num_channels, hf_llrp_channel, 2, suboffset);
        break;
    case LLRP_IMPINJ_PARAM_REDUCED_POWER_FREQUENCY_LIST:
        PARAM_TREE_ADD(impinj_reduce_power_mode, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(rfu, 2, ENC_NA);
        suboffset = dissect_llrp_item_array(tvb, pinfo, param_tree,
                hf_llrp_num_channels, hf_llrp_channel, 2, suboffset);
        break;
    case LLRP_IMPINJ_PARAM_LOW_DUTY_CYCLE:
        PARAM_TREE_ADD(impinj_low_duty_mode, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(empty_field_timeout, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(field_ping_interval, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_DETAILED_VERSION:
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_model_name, suboffset);
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_serial_number, suboffset);
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_soft_ver, suboffset);
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_firm_ver, suboffset);
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_fpga_ver, suboffset);
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_pcba_ver, suboffset);
        break;
    case LLRP_IMPINJ_PARAM_FREQUENCY_CAPABILITIES:
        suboffset = dissect_llrp_item_array(tvb, pinfo, param_tree,
                hf_llrp_num_freqs, hf_llrp_frequency, 4, suboffset);
        break;
    case LLRP_IMPINJ_PARAM_FORKLIFT_HEIGHT_THRESHOLD:
        PARAM_TREE_ADD(height_thresh, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_FORKLIFT_ZEROMOTION_TIME_THRESHOLD:
        PARAM_TREE_ADD(zero_motion_thresh, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_FORKLIFT_COMPANION_BOARD_INFO:
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_board_manufacturer, suboffset);
        PARAM_TREE_ADD(fw_ver_hex, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(hw_ver_hex, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_GPI_DEBOUNCE_CONFIGURATION:
        PARAM_TREE_ADD(gpi_port, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(gpi_debounce, 4, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_READER_TEMPERATURE:
        PARAM_TREE_ADD(temperature, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_LINK_MONITOR_CONFIGURATION:
        PARAM_TREE_ADD(impinj_link_monitor_mode, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(link_down_thresh, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_REPORT_BUFFER_CONFIGURATION:
        PARAM_TREE_ADD(impinj_report_buff_mode, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_BLOCK_WRITE_WORD_COUNT:
        PARAM_TREE_ADD(word_count, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_BLOCK_PERMALOCK:
        PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(access_pass, 4, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(mb, 1, ENC_NA);
        PARAM_TREE_ADD(block_pointer, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(block_mask, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_BLOCK_PERMALOCK_OPSPEC_RESULT:
        PARAM_TREE_ADD(permalock_result, 1, ENC_NA);
        PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_GET_BLOCK_PERMALOCK_STATUS:
        PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(access_pass, 4, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(mb, 1, ENC_NA);
        PARAM_TREE_ADD(block_pointer, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(block_range, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_GET_BLOCK_PERMALOCK_STATUS_OPSPEC_RESULT:
        PARAM_TREE_ADD(block_permalock_result, 1, ENC_NA);
        PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_SET_QT_CONFIG:
        PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(access_pass, 4, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(impinj_data_profile, 1, ENC_NA);
        PARAM_TREE_ADD(impinj_access_range, 1, ENC_NA);
        PARAM_TREE_ADD(impinj_persistence, 1, ENC_NA);
        PARAM_TREE_ADD(rfu, 4, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_SET_QT_CONFIG_OPSPEC_RESULT:
        PARAM_TREE_ADD(set_qt_config_result, 1, ENC_NA);
        PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_GET_QT_CONFIG:
        PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(access_pass, 4, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_GET_QT_CONFIG_OPSPEC_RESULT:
        PARAM_TREE_ADD(get_qt_config_result, 1, ENC_NA);
        PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(impinj_data_profile, 1, ENC_NA);
        PARAM_TREE_ADD(impinj_access_range, 1, ENC_NA);
        PARAM_TREE_ADD(rfu, 4, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_ENABLE_SERIALIZED_TID:
        PARAM_TREE_ADD(impinj_serialized_tid_mode, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_ENABLE_RF_PHASE_ANGLE:
        PARAM_TREE_ADD(impinj_rf_phase_mode, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_ENABLE_PEAK_RSSI:
        PARAM_TREE_ADD(impinj_peak_rssi_mode, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_ENABLE_GPS_COORDINATES:
        PARAM_TREE_ADD(impinj_gps_coordinates_mode, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_SERIALIZED_TID:
        PARAM_TREE_ADD(impinj_tid, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_RF_PHASE_ANGLE:
        PARAM_TREE_ADD(phase_angle, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_PEAK_RSSI:
        PARAM_TREE_ADD(rssi, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_GPS_COORDINATES:
        PARAM_TREE_ADD(latitude, 4, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(longitude, 4, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_LOOP_SPEC:
        PARAM_TREE_ADD(loop_count, 4, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_GGA_SENTENCE:
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_gga_sentence, suboffset);
        break;
    case LLRP_IMPINJ_PARAM_RMC_SENTENCE:
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_rmc_sentence, suboffset);
        break;
    case LLRP_IMPINJ_PARAM_OPSPEC_RETRY_COUNT:
        PARAM_TREE_ADD(retry_count, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_ADVANCE_GPO_CONFIG:
        PARAM_TREE_ADD(gpo_port, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(impinj_gpo_mode, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(gpo_pulse_dur, 4, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_ENABLE_OPTIM_READ:
        PARAM_TREE_ADD(impinj_optim_read_mode, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_ACCESS_SPEC_ORDERING:
        PARAM_TREE_ADD(impinj_access_spec_ordering, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_ENABLE_RF_DOPPLER_FREQ:
        PARAM_TREE_ADD(impinj_rf_doppler_mode, 2, ENC_BIG_ENDIAN);
        break;
    case LLRP_IMPINJ_PARAM_ARRAY_VERSION:
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_serial_number, suboffset);
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_firm_ver, suboffset);
        suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_pcba_ver, suboffset);
        break;
    case LLRP_IMPINJ_PARAM_HUB_CONFIGURATION:
        PARAM_TREE_ADD(impinj_hub_id, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(impinj_hub_connected_type, 2, ENC_BIG_ENDIAN);
        PARAM_TREE_ADD(impinj_hub_fault_type, 2, ENC_BIG_ENDIAN);
        break;
    default:
        return suboffset;
        break;
    }
    /
    return dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
}
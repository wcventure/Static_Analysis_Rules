static guint
dissect_llrp_parameters(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree,
        guint offset, const guint end)
{
    guint8      has_length;
    guint16     len, type;
    guint       real_len, param_end;
    guint       suboffset;
    guint       num;
    proto_item *ti;
    proto_tree *param_tree;

    while (((gint)(end - offset)) > 0)
    {
        has_length = !(tvb_get_guint8(tvb, offset) & 0x80);

        if (has_length)
        {
            type = tvb_get_ntohs(tvb, offset);
            len = tvb_get_ntohs(tvb, offset + 2);

            if (len < LLRP_TLV_LEN_MIN)
                real_len = LLRP_TLV_LEN_MIN;
            else if (len > tvb_reported_length_remaining(tvb, offset))
                real_len = tvb_reported_length_remaining(tvb, offset);
            else
                real_len = len;

            param_end = offset + real_len;

            ti = proto_tree_add_none_format(tree, hf_llrp_param, tvb,
                    offset, real_len, "TLV Parameter: %s",
                    val_to_str_ext(type, &tlv_type_ext, "Unknown Type: %d"));
            param_tree = proto_item_add_subtree(ti, ett_llrp_param);

            proto_tree_add_item(param_tree, hf_llrp_tlv_type, tvb,
                    offset, 2, ENC_BIG_ENDIAN);
            offset += 2;

            ti = proto_tree_add_item(param_tree, hf_llrp_tlv_len, tvb,
                    offset, 2, ENC_BIG_ENDIAN);
            if (len != real_len)
                expert_add_info_format(pinfo, ti, &ei_llrp_invalid_length,
                        "Invalid length field: claimed %u, should be %u.",
                        len, real_len);
            offset += 2;

            suboffset = offset;
            switch(type) {
            case LLRP_TLV_RO_BOUND_SPEC:
            case LLRP_TLV_UHF_CAPABILITIES:
            case LLRP_TLV_ACCESS_COMMAND:
            case LLRP_TLV_TAG_REPORT_DATA:
            case LLRP_TLV_RF_SURVEY_REPORT_DATA:
            case LLRP_TLV_READER_EVENT_NOTI_SPEC:
            case LLRP_TLV_READER_EVENT_NOTI_DATA:
            case LLRP_TLV_C1G2_UHF_RF_MD_TBL:
            case LLRP_TLV_C1G2_TAG_SPEC:
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_UTC_TIMESTAMP:
            case LLRP_TLV_UPTIME:
                PARAM_TREE_ADD(microseconds, 8, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_GENERAL_DEVICE_CAP:
                PARAM_TREE_ADD_STAY(max_supported_antenna, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(can_set_antenna_prop, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(has_utc_clock, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(device_manufacturer, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(model, 4, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_firmware_version, suboffset);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_MAX_RECEIVE_SENSE:
                PARAM_TREE_ADD(max_receive_sense, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_RECEIVE_SENSE_ENTRY:
                PARAM_TREE_ADD(index, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(receive_sense, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_ANTENNA_RCV_SENSE_RANGE:
                PARAM_TREE_ADD(antenna_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(receive_sense_index_min, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(receive_sense_index_max, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_ANTENNA_AIR_PROTO:
                PARAM_TREE_ADD(antenna_id, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_item_array(tvb, pinfo, param_tree,
                        hf_llrp_num_protocols, hf_llrp_protocol_id, 1, suboffset);
                break;
            case LLRP_TLV_GPIO_CAPABILITIES:
                PARAM_TREE_ADD(num_gpi, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(num_gpo, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_LLRP_CAPABILITIES:
                PARAM_TREE_ADD_STAY(can_do_survey, 1, ENC_NA);
                PARAM_TREE_ADD_STAY(can_report_buffer_warning, 1, ENC_NA);
                PARAM_TREE_ADD_STAY(support_client_opspec, 1, ENC_NA);
                PARAM_TREE_ADD_STAY(can_stateaware, 1, ENC_NA);
                PARAM_TREE_ADD(support_holding, 1, ENC_NA);
                PARAM_TREE_ADD(max_priority_supported, 1, ENC_NA);
                PARAM_TREE_ADD(client_opspec_timeout, 2, ENC_BIG_ENDIAN);
                num = tvb_get_ntohl(tvb, suboffset);
                if(num == LLRP_NO_LIMIT)
                    PARAM_TREE_ADD_SPEC_STAY(uint_format_value, max_num_rospec, 4, num, "No limit (%u)");
                else
                    PARAM_TREE_ADD_STAY(max_num_rospec, 4, ENC_BIG_ENDIAN);
                suboffset += 4;
                num = tvb_get_ntohl(tvb, suboffset);
                if(num == LLRP_NO_LIMIT)
                    PARAM_TREE_ADD_SPEC_STAY(uint_format_value, max_num_spec_per_rospec, 4, num, "No limit (%u)");
                else
                    PARAM_TREE_ADD_STAY(max_num_spec_per_rospec, 4, ENC_BIG_ENDIAN);
                suboffset += 4;
                num = tvb_get_ntohl(tvb, suboffset);
                if(num == LLRP_NO_LIMIT)
                    PARAM_TREE_ADD_SPEC_STAY(uint_format_value, max_num_inventory_per_aispec, 4, num, "No limit (%u)");
                else
                    PARAM_TREE_ADD_STAY(max_num_inventory_per_aispec, 4, ENC_BIG_ENDIAN);
                suboffset += 4;
                num = tvb_get_ntohl(tvb, suboffset);
                if(num == LLRP_NO_LIMIT)
                    PARAM_TREE_ADD_SPEC_STAY(uint_format_value, max_num_accessspec, 4, num, "No limit (%u)");
                else
                    PARAM_TREE_ADD_STAY(max_num_accessspec, 4, ENC_BIG_ENDIAN);
                suboffset += 4;
                num = tvb_get_ntohl(tvb, suboffset);
                if(num == LLRP_NO_LIMIT)
                    PARAM_TREE_ADD_SPEC_STAY(uint_format_value, max_num_opspec_per_accressspec, 4, num, "No limit (%u)");
                else
                    PARAM_TREE_ADD_STAY(max_num_opspec_per_accressspec, 4, ENC_BIG_ENDIAN);
                suboffset += 4;
                break;
            case LLRP_TLV_REGU_CAPABILITIES:
                PARAM_TREE_ADD(country_code, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(comm_standard, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_XMIT_POWER_LEVEL_ENTRY:
                PARAM_TREE_ADD(index, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(transmit_power, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_FREQ_INFORMATION:
                PARAM_TREE_ADD(hopping, 1, ENC_NA);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_FREQ_HOP_TABLE:
                PARAM_TREE_ADD(hop_table_id, 1, ENC_NA);
                PARAM_TREE_ADD(rfu, 1, ENC_NA);
                suboffset = dissect_llrp_item_array(tvb, pinfo, param_tree,
                        hf_llrp_num_hops, hf_llrp_frequency, 4, suboffset);
                break;
            case LLRP_TLV_FIXED_FREQ_TABLE:
                suboffset = dissect_llrp_item_array(tvb, pinfo, param_tree,
                        hf_llrp_num_freqs, hf_llrp_frequency, 4, suboffset);
                break;
            case LLRP_TLV_RF_SURVEY_FREQ_CAP:
                PARAM_TREE_ADD(min_freq, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(max_freq, 4, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_RO_SPEC:
                PARAM_TREE_ADD(rospec_id, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(priority, 1, ENC_NA);
                PARAM_TREE_ADD(cur_state, 1, ENC_NA);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_RO_SPEC_START_TRIGGER:
                PARAM_TREE_ADD(rospec_start_trig_type, 1, ENC_NA);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_PER_TRIGGER_VAL:
                PARAM_TREE_ADD(offset, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(period, 4, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_GPI_TRIGGER_VAL:
                PARAM_TREE_ADD(gpi_port, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(gpi_event, 1, ENC_NA);
                PARAM_TREE_ADD(timeout, 4, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_RO_SPEC_STOP_TRIGGER:
                PARAM_TREE_ADD(rospec_stop_trig_type, 1, ENC_NA);
                PARAM_TREE_ADD(duration_trig, 4, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_AI_SPEC:
                suboffset = dissect_llrp_item_array(tvb, pinfo, param_tree,
                        hf_llrp_antenna_count, hf_llrp_antenna, 2, suboffset);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_AI_SPEC_STOP:
                PARAM_TREE_ADD(aispec_stop_trig_type, 1, ENC_NA);
                PARAM_TREE_ADD(duration_trig, 4, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_TAG_OBSERV_TRIGGER:
                PARAM_TREE_ADD(trig_type, 1, ENC_NA);
                PARAM_TREE_ADD(rfu, 1, ENC_NA);
                PARAM_TREE_ADD(number_of_tags, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(number_of_attempts, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(t, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(timeout, 4, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_INVENTORY_PARAM_SPEC:
                PARAM_TREE_ADD(inventory_spec_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(protocol_id, 1, ENC_NA);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_RF_SURVEY_SPEC:
                PARAM_TREE_ADD(antenna_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(start_freq, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(stop_freq, 4, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_RF_SURVEY_SPEC_STOP_TR:
                PARAM_TREE_ADD(stop_trig_type, 1, ENC_NA);
                PARAM_TREE_ADD(duration, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(n_4, 4, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_LOOP_SPEC:
                PARAM_TREE_ADD(loop_count, 4, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_ACCESS_SPEC:
                PARAM_TREE_ADD(accessspec_id, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(antenna_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(protocol_id, 1, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(access_cur_state, 1, ENC_NA);
                PARAM_TREE_ADD(rospec_id, 4, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_ACCESS_SPEC_STOP_TRIG:
                PARAM_TREE_ADD(access_stop_trig_type, 1, ENC_NA);
                PARAM_TREE_ADD(operation_count, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_CLIENT_REQ_OP_SPEC:
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_CLIENT_REQ_RESPONSE:
                PARAM_TREE_ADD(accessspec_id, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_LLRP_CONF_STATE_VAL:
                PARAM_TREE_ADD(conf_value, 4, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_IDENT:
                PARAM_TREE_ADD(id_type, 1, ENC_NA);
                num = tvb_get_ntohs(tvb, suboffset);
                PARAM_TREE_ADD(reader_id, 2, ENC_BIG_ENDIAN);
                suboffset += num;
                break;
            case LLRP_TLV_GPO_WRITE_DATA:
                PARAM_TREE_ADD(gpo_port, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(gpo_data, 1, ENC_NA);
                break;
            case LLRP_TLV_KEEPALIVE_SPEC:
                PARAM_TREE_ADD(keepalive_trig_type, 1, ENC_NA);
                PARAM_TREE_ADD(time_iterval, 4, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_ANTENNA_PROPS:
                PARAM_TREE_ADD(antenna_connected, 1, ENC_NA);
                PARAM_TREE_ADD(antenna_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(antenna_gain, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_ANTENNA_CONF:
                PARAM_TREE_ADD(antenna_id, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_RF_RECEIVER:
                PARAM_TREE_ADD(receiver_sense, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_RF_TRANSMITTER:
                PARAM_TREE_ADD(hop_table_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(channel_idx, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(transmit_power, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_GPI_PORT_CURRENT_STATE:
                PARAM_TREE_ADD(gpi_port, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(gpi_config, 1, ENC_NA);
                PARAM_TREE_ADD(gpi_state, 1, ENC_NA);
                break;
            case LLRP_TLV_EVENTS_AND_REPORTS:
                PARAM_TREE_ADD(hold_events_and_reports, 1, ENC_NA);
                break;
            case LLRP_TLV_RO_REPORT_SPEC:
                PARAM_TREE_ADD(ro_report_trig, 1, ENC_NA);
                PARAM_TREE_ADD(n_2, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_TAG_REPORT_CONTENT_SEL:
                PARAM_TREE_ADD_STAY(enable_rospec_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD_STAY(enable_spec_idx, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD_STAY(enable_inv_spec_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD_STAY(enable_antenna_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD_STAY(enable_channel_idx, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD_STAY(enable_peak_rssi, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD_STAY(enable_first_seen, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD_STAY(enable_last_seen, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD_STAY(enable_seen_count, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(enable_accessspec_id, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_ACCESS_REPORT_SPEC:
                PARAM_TREE_ADD(access_report_trig, 1, ENC_NA);
                break;
            case LLRP_TLV_EPC_DATA:
                suboffset = dissect_llrp_bit_field(tvb, param_tree, hf_llrp_epc, suboffset);
                break;
            case LLRP_TLV_FREQ_RSSI_LEVEL_ENTRY:
                PARAM_TREE_ADD(frequency, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(bandwidth, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(average_rssi, 1, ENC_NA);
                PARAM_TREE_ADD(peak_rssi, 1, ENC_NA);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_EVENT_NOTIF_STATE:
                PARAM_TREE_ADD(event_type, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(notif_state, 1, ENC_NA);
                break;
            case LLRP_TLV_HOPPING_EVENT:
                PARAM_TREE_ADD(hop_table_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(next_chan_idx, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_GPI_EVENT:
                PARAM_TREE_ADD(gpi_port, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(gpi_event, 1, ENC_NA);
                break;
            case LLRP_TLV_RO_SPEC_EVENT:
                PARAM_TREE_ADD(roevent_type, 1, ENC_NA);
                PARAM_TREE_ADD(rospec_id, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(prem_rospec_id, 4, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_REPORT_BUF_LEVEL_WARN:
                PARAM_TREE_ADD(buffer_full_percentage, 1, ENC_NA);
                break;
            case LLRP_TLV_REPORT_BUF_OVERFLOW_ERR:
                break;
            case LLRP_TLV_READER_EXCEPTION_EVENT:
                suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_message, suboffset);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_RF_SURVEY_EVENT:
                PARAM_TREE_ADD(rfevent_type, 1, ENC_NA);
                PARAM_TREE_ADD(rospec_id, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(spec_idx, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_AI_SPEC_EVENT:
                PARAM_TREE_ADD(aievent_type, 1, ENC_NA);
                PARAM_TREE_ADD(rospec_id, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(spec_idx, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_ANTENNA_EVENT:
                PARAM_TREE_ADD(antenna_event_type, 1, ENC_NA);
                PARAM_TREE_ADD(antenna_id, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_CONN_ATTEMPT_EVENT:
                PARAM_TREE_ADD(conn_status, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_CONN_CLOSE_EVENT:
                break;
            case LLRP_TLV_SPEC_LOOP_EVENT:
                PARAM_TREE_ADD(rospec_id, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(loop_count, 4, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_LLRP_STATUS:
                PARAM_TREE_ADD(status_code, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_utf8_parameter(tvb, pinfo, param_tree, hf_llrp_error_desc, suboffset);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_FIELD_ERROR:
                PARAM_TREE_ADD(field_num, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(error_code, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_PARAM_ERROR:
                PARAM_TREE_ADD(parameter_type, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(error_code, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_C1G2_LLRP_CAP:
                PARAM_TREE_ADD_STAY(can_support_block_erase, 1, ENC_NA);
                PARAM_TREE_ADD_STAY(can_support_block_write, 1, ENC_NA);
                PARAM_TREE_ADD_STAY(can_support_block_permalock, 1, ENC_NA);
                PARAM_TREE_ADD_STAY(can_support_tag_recomm, 1, ENC_NA);
                PARAM_TREE_ADD_STAY(can_support_UMI_method2, 1, ENC_NA);
                PARAM_TREE_ADD(can_support_XPC, 1, ENC_NA);
                num = tvb_get_ntohs(tvb, suboffset);
                if(num == LLRP_NO_LIMIT)
                    PARAM_TREE_ADD_SPEC_STAY(uint_format_value, max_num_filter_per_query, 2, num, "No limit (%u)");
                else
                    PARAM_TREE_ADD_STAY(max_num_filter_per_query, 2, ENC_BIG_ENDIAN);
                suboffset += 2;
                break;
            case LLRP_TLV_C1G2_UHF_RF_MD_TBL_ENT:
                PARAM_TREE_ADD(mode_ident, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD_STAY(DR, 1, ENC_NA);
                PARAM_TREE_ADD(hag_conformance, 1, ENC_NA);
                PARAM_TREE_ADD(mod, 1, ENC_NA);
                PARAM_TREE_ADD(flm, 1, ENC_NA);
                PARAM_TREE_ADD(m, 1, ENC_NA);
                PARAM_TREE_ADD(bdr, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(pie, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(min_tari, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(max_tari, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(step_tari, 4, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_C1G2_INVENTORY_COMMAND:
                PARAM_TREE_ADD(inventory_state_aware, 1, ENC_NA);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_C1G2_FILTER:
                PARAM_TREE_ADD(trunc, 1, ENC_NA);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_C1G2_TAG_INV_MASK:
                PARAM_TREE_ADD(mb, 1, ENC_NA);
                PARAM_TREE_ADD(pointer, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_bit_field(tvb, param_tree, hf_llrp_tag_mask, suboffset);
                break;
            case LLRP_TLV_C1G2_TAG_INV_AWARE_FLTR:
                PARAM_TREE_ADD(aware_filter_target, 1, ENC_NA);
                PARAM_TREE_ADD(aware_filter_action, 1, ENC_NA);
                break;
            case LLRP_TLV_C1G2_TAG_INV_UNAWR_FLTR:
                PARAM_TREE_ADD(unaware_filter_action, 1, ENC_NA);
                break;
            case LLRP_TLV_C1G2_RF_CONTROL:
                PARAM_TREE_ADD(mode_idx, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(tari, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_C1G2_SINGULATION_CTRL:
                PARAM_TREE_ADD(session, 1, ENC_NA);
                PARAM_TREE_ADD(tag_population, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(tag_transit_time, 4, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_C1G2_TAG_INV_AWARE_SING:
                PARAM_TREE_ADD_STAY(sing_i, 1, ENC_NA);
                PARAM_TREE_ADD_STAY(sing_s, 1, ENC_NA);
                PARAM_TREE_ADD(sing_a, 1, ENC_NA);
                break;
            case LLRP_TLV_C1G2_TARGET_TAG:
                PARAM_TREE_ADD_STAY(mb, 1, ENC_NA);
                PARAM_TREE_ADD(match, 1, ENC_NA);
                PARAM_TREE_ADD(pointer, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_bit_field(tvb, param_tree, hf_llrp_tag_mask, suboffset);
                suboffset = dissect_llrp_bit_field(tvb, param_tree, hf_llrp_tag_data, suboffset);
                break;
            case LLRP_TLV_C1G2_READ:
            case LLRP_TLV_C1G2_BLK_ERASE:
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(access_pass, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(mb, 1, ENC_NA);
                PARAM_TREE_ADD(word_pointer, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(word_count, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_C1G2_WRITE:
            case LLRP_TLV_C1G2_BLK_WRITE:
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(access_pass, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(mb, 1, ENC_NA);
                PARAM_TREE_ADD(word_pointer, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_word_array(tvb, param_tree, hf_llrp_write_data, suboffset);
                break;
            case LLRP_TLV_C1G2_KILL:
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(kill_pass, 4, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_C1G2_RECOMMISSION:
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(kill_pass, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD_STAY(kill_3, 1, ENC_NA);
                PARAM_TREE_ADD_STAY(kill_2, 1, ENC_NA);
                PARAM_TREE_ADD(kill_l, 1, ENC_NA);
                break;
            case LLRP_TLV_C1G2_LOCK:
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(access_pass, 4, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_parameters(tvb, pinfo, param_tree, suboffset, param_end);
                break;
            case LLRP_TLV_C1G2_LOCK_PAYLOAD:
                PARAM_TREE_ADD(privilege, 1, ENC_NA);
                PARAM_TREE_ADD(data_field, 1, ENC_NA);
                break;
            case LLRP_TLV_C1G2_BLK_PERMALOCK:
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(access_pass, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(mb, 1, ENC_NA);
                PARAM_TREE_ADD(block_pointer, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_word_array(tvb, param_tree, hf_llrp_block_mask, suboffset);
                break;
            case LLRP_TLV_C1G2_GET_BLK_PERMALOCK:
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(access_pass, 4, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(mb, 1, ENC_NA);
                PARAM_TREE_ADD(block_pointer, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(block_range, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_C1G2_EPC_MEMORY_SLCTOR:
                PARAM_TREE_ADD_STAY(enable_crc, 1, ENC_NA);
                PARAM_TREE_ADD_STAY(enable_pc, 1, ENC_NA);
                PARAM_TREE_ADD(enable_xpc, 1, ENC_NA);
                break;
            case LLRP_TLV_C1G2_READ_OP_SPEC_RES:
                PARAM_TREE_ADD(access_result, 1, ENC_NA);
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_word_array(tvb, param_tree, hf_llrp_read_data, suboffset);
                break;
            case LLRP_TLV_C1G2_WRT_OP_SPEC_RES:
            case LLRP_TLV_C1G2_BLK_WRT_OP_SPC_RES:
                PARAM_TREE_ADD(access_result, 1, ENC_NA);
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                PARAM_TREE_ADD(num_words_written, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_C1G2_KILL_OP_SPEC_RES:
            case LLRP_TLV_C1G2_RECOM_OP_SPEC_RES:
            case LLRP_TLV_C1G2_LOCK_OP_SPEC_RES:
            case LLRP_TLV_C1G2_BLK_ERS_OP_SPC_RES:
            case LLRP_TLV_C1G2_BLK_PRL_OP_SPC_RES:
                PARAM_TREE_ADD(access_result, 1, ENC_NA);
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                break;
            case LLRP_TLV_C1G2_BLK_PRL_STAT_RES:
                PARAM_TREE_ADD(access_result, 1, ENC_NA);
                PARAM_TREE_ADD(opspec_id, 2, ENC_BIG_ENDIAN);
                suboffset = dissect_llrp_word_array(tvb, param_tree, hf_llrp_permlock_status, suboffset);
                break;
            case LLRP_TLV_CUSTOM_PARAMETER:
                num = tvb_get_ntohl(tvb, suboffset);
                PARAM_TREE_ADD(vendor_id, 4, ENC_BIG_ENDIAN);
                switch(num) {
                case LLRP_VENDOR_IMPINJ:
                    suboffset = dissect_llrp_impinj_parameter(tvb, pinfo, param_tree, suboffset, param_end);
                    break;
                }
                break;
            }
            /
            if(suboffset != param_end) {
                /
                expert_add_info_format(pinfo, param_tree, &ei_llrp_invalid_length,
                        "Incorrect length of parameter: %u bytes decoded, but %u bytes claimed.",
                        suboffset - offset + 4, real_len);
            }
            /
            offset += real_len - 4;
        }
        else
        {
            type = tvb_get_guint8(tvb, offset) & 0x7F;

            switch (type)
            {
                case LLRP_TV_ANTENNA_ID:
                    real_len = LLRP_TV_LEN_ANTENNA_ID; break;
                case LLRP_TV_FIRST_SEEN_TIME_UTC:
                    real_len = LLRP_TV_LEN_FIRST_SEEN_TIME_UTC; break;
                case LLRP_TV_FIRST_SEEN_TIME_UPTIME:
                    real_len = LLRP_TV_LEN_FIRST_SEEN_TIME_UPTIME; break;
                case LLRP_TV_LAST_SEEN_TIME_UTC:
                    real_len = LLRP_TV_LEN_LAST_SEEN_TIME_UTC; break;
                case LLRP_TV_LAST_SEEN_TIME_UPTIME:
                    real_len = LLRP_TV_LEN_LAST_SEEN_TIME_UPTIME; break;
                case LLRP_TV_PEAK_RSSI:
                    real_len = LLRP_TV_LEN_PEAK_RSSI; break;
                case LLRP_TV_CHANNEL_INDEX:
                    real_len = LLRP_TV_LEN_CHANNEL_INDEX; break;
                case LLRP_TV_TAG_SEEN_COUNT:
                    real_len = LLRP_TV_LEN_TAG_SEEN_COUNT; break;
                case LLRP_TV_RO_SPEC_ID:
                    real_len = LLRP_TV_LEN_RO_SPEC_ID; break;
                case LLRP_TV_INVENTORY_PARAM_SPEC_ID:
                    real_len = LLRP_TV_LEN_INVENTORY_PARAM_SPEC_ID; break;
                case LLRP_TV_C1G2_CRC:
                    real_len = LLRP_TV_LEN_C1G2_CRC; break;
                case LLRP_TV_C1G2_PC:
                    real_len = LLRP_TV_LEN_C1G2_PC; break;
                case LLRP_TV_EPC96:
                    real_len = LLRP_TV_LEN_EPC96; break;
                case LLRP_TV_SPEC_INDEX:
                    real_len = LLRP_TV_LEN_SPEC_INDEX; break;
                case LLRP_TV_CLIENT_REQ_OP_SPEC_RES:
                    real_len = LLRP_TV_LEN_CLIENT_REQ_OP_SPEC_RES; break;
                case LLRP_TV_ACCESS_SPEC_ID:
                    real_len = LLRP_TV_LEN_ACCESS_SPEC_ID; break;
                case LLRP_TV_OP_SPEC_ID:
                    real_len = LLRP_TV_LEN_OP_SPEC_ID; break;
                case LLRP_TV_C1G2_SINGULATION_DET:
                    real_len = LLRP_TV_LEN_C1G2_SINGULATION_DET; break;
                case LLRP_TV_C1G2_XPC_W1:
                    real_len = LLRP_TV_LEN_C1G2_XPC_W1; break;
                case LLRP_TV_C1G2_XPC_W2:
                    real_len = LLRP_TV_LEN_C1G2_XPC_W2; break;
                default:
                    /
                    real_len = 0;
                    break;
            };

            ti = proto_tree_add_none_format(tree, hf_llrp_param, tvb,
                    offset, real_len + 1, "TV Parameter : %s",
                    val_to_str_ext(type, &tv_type_ext, "Unknown Type: %d"));
            param_tree = proto_item_add_subtree(ti, ett_llrp_param);

            proto_tree_add_item(param_tree, hf_llrp_tv_type, tvb,
                    offset, 1, ENC_BIG_ENDIAN);
            offset++;

            suboffset = offset;
            switch (type)
            {
                case LLRP_TV_ANTENNA_ID:
                    PARAM_TREE_ADD_STAY(antenna_id, 2, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_FIRST_SEEN_TIME_UTC:
                case LLRP_TV_FIRST_SEEN_TIME_UPTIME:
                case LLRP_TV_LAST_SEEN_TIME_UTC:
                case LLRP_TV_LAST_SEEN_TIME_UPTIME:
                    PARAM_TREE_ADD_STAY(microseconds, 8, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_PEAK_RSSI:
                    PARAM_TREE_ADD_STAY(peak_rssi, 1, ENC_NA);
                    break;
                case LLRP_TV_CHANNEL_INDEX:
                    PARAM_TREE_ADD_STAY(channel_idx, 2, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_TAG_SEEN_COUNT:
                    PARAM_TREE_ADD_STAY(tag_count, 2, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_RO_SPEC_ID:
                    PARAM_TREE_ADD_STAY(rospec_id, 4, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_INVENTORY_PARAM_SPEC_ID:
                    PARAM_TREE_ADD_STAY(inventory_spec_id, 2, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_C1G2_CRC:
                    PARAM_TREE_ADD_STAY(crc, 2, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_C1G2_PC:
                    PARAM_TREE_ADD_STAY(pc_bits, 2, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_EPC96:
                    PARAM_TREE_ADD_STAY(epc, 96/8, ENC_NA);
                    break;
                case LLRP_TV_SPEC_INDEX:
                    PARAM_TREE_ADD_STAY(spec_idx, 2, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_CLIENT_REQ_OP_SPEC_RES:
                    PARAM_TREE_ADD_STAY(opspec_id, 2, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_ACCESS_SPEC_ID:
                    PARAM_TREE_ADD_STAY(accessspec_id, 4, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_OP_SPEC_ID:
                    PARAM_TREE_ADD_STAY(opspec_id, 2, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_C1G2_SINGULATION_DET:
                    PARAM_TREE_ADD_STAY(num_coll, 2, ENC_BIG_ENDIAN);
                    PARAM_TREE_ADD_STAY(num_empty, 2, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_C1G2_XPC_W1:
                    PARAM_TREE_ADD_STAY(xpc_w1, 2, ENC_BIG_ENDIAN);
                    break;
                case LLRP_TV_C1G2_XPC_W2:
                    PARAM_TREE_ADD_STAY(xpc_w2, 2, ENC_BIG_ENDIAN);
                    break;
            };
            /
            offset += real_len;
        }
    }
    return offset;
}
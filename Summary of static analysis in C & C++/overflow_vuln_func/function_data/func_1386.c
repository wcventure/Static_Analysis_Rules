static void
dissect_llrp_message(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree,
        guint16 type, guint offset)
{
    gboolean    ends_with_parameters;
    guint8      requested_data;
    guint16     antenna_id, gpi_port, gpo_port;
    guint32     spec_id, vendor;
    proto_item *request_item, *antenna_item, *gpi_item, *gpo_item;
    guint (*dissect_custom_message)(tvbuff_t *tvb,
            packet_info *pinfo, proto_tree *tree, guint offset) = NULL;

    ends_with_parameters = FALSE;
    switch (type)
    {
        /
        case LLRP_TYPE_CLOSE_CONNECTION_RESPONSE:
        case LLRP_TYPE_GET_READER_CAPABILITIES_RESPONSE:
        case LLRP_TYPE_ADD_ROSPEC:
        case LLRP_TYPE_ADD_ROSPEC_RESPONSE:
        case LLRP_TYPE_DELETE_ROSPEC_RESPONSE:
        case LLRP_TYPE_START_ROSPEC_RESPONSE:
        case LLRP_TYPE_STOP_ROSPEC_RESPONSE:
        case LLRP_TYPE_ENABLE_ROSPEC_RESPONSE:
        case LLRP_TYPE_DISABLE_ROSPEC_RESPONSE:
        case LLRP_TYPE_GET_ROSPECS_RESPONSE:
        case LLRP_TYPE_ADD_ACCESSSPEC:
        case LLRP_TYPE_ADD_ACCESSSPEC_RESPONSE:
        case LLRP_TYPE_DELETE_ACCESSSPEC_RESPONSE:
        case LLRP_TYPE_ENABLE_ACCESSSPEC_RESPONSE:
        case LLRP_TYPE_DISABLE_ACCESSSPEC_RESPONSE:
        case LLRP_TYPE_GET_ACCESSSPECS:
        case LLRP_TYPE_CLIENT_REQUEST_OP:
        case LLRP_TYPE_CLIENT_RESQUEST_OP_RESPONSE:
        case LLRP_TYPE_RO_ACCESS_REPORT:
        case LLRP_TYPE_READER_EVENT_NOTIFICATION:
        case LLRP_TYPE_ERROR_MESSAGE:
        case LLRP_TYPE_GET_READER_CONFIG_RESPONSE:
        case LLRP_TYPE_SET_READER_CONFIG_RESPONSE:
        case LLRP_TYPE_SET_PROTOCOL_VERSION_RESPONSE:
        case LLRP_TYPE_GET_ACCESSSPECS_RESPONSE:
        case LLRP_TYPE_GET_REPORT:
        case LLRP_TYPE_ENABLE_EVENTS_AND_REPORTS:
            ends_with_parameters = TRUE;
            break;
        /
        case LLRP_TYPE_START_ROSPEC:
        case LLRP_TYPE_STOP_ROSPEC:
        case LLRP_TYPE_ENABLE_ROSPEC:
        case LLRP_TYPE_DISABLE_ROSPEC:
        case LLRP_TYPE_DELETE_ROSPEC:
            spec_id = tvb_get_ntohl(tvb, offset);
            if (spec_id == LLRP_ROSPEC_ALL)
                proto_tree_add_uint_format(tree, hf_llrp_rospec, tvb,
                        offset, 4, spec_id, "All ROSpecs (%u)", spec_id);
            else
                proto_tree_add_item(tree, hf_llrp_rospec, tvb,
                        offset, 4, ENC_BIG_ENDIAN);
            offset += 4;
            break;
        /
        case LLRP_TYPE_ENABLE_ACCESSSPEC:
        case LLRP_TYPE_DELETE_ACCESSSPEC:
        case LLRP_TYPE_DISABLE_ACCESSSPEC:
            spec_id = tvb_get_ntohl(tvb, offset);
            if (spec_id == LLRP_ACCESSSPEC_ALL)
                proto_tree_add_uint_format(tree, hf_llrp_accessspec, tvb,
                        offset, 4, spec_id, "All Access Specs (%u)", spec_id);
            else
                proto_tree_add_item(tree, hf_llrp_accessspec, tvb,
                        offset, 4, ENC_BIG_ENDIAN);
            offset += 4;
            break;
        case LLRP_TYPE_GET_READER_CAPABILITIES:
            proto_tree_add_item(tree, hf_llrp_req_cap, tvb, offset, 1, ENC_BIG_ENDIAN);
            offset++;
            ends_with_parameters = TRUE;
            break;
        /
        case LLRP_TYPE_GET_READER_CONFIG:
            antenna_id = tvb_get_ntohs(tvb, offset);
            if (antenna_id == LLRP_ANTENNA_ALL)
                antenna_item = proto_tree_add_uint_format(tree, hf_llrp_antenna_id, tvb,
                        offset, 2, antenna_id, "All Antennas (%u)", antenna_id);
            else
                antenna_item = proto_tree_add_item(tree, hf_llrp_antenna_id, tvb,
                        offset, 2, ENC_BIG_ENDIAN);
            offset += 2;

            requested_data = tvb_get_guint8(tvb, offset);
            request_item = proto_tree_add_item(tree, hf_llrp_req_conf, tvb,
                    offset, 1, ENC_BIG_ENDIAN);
            offset++;

            gpi_port = tvb_get_ntohs(tvb, offset);
            if (gpi_port == LLRP_GPI_PORT_ALL)
                gpi_item = proto_tree_add_uint_format(tree, hf_llrp_gpi_port, tvb,
                        offset, 2, gpi_port, "All GPI Ports (%u)", gpi_port);
            else
                gpi_item = proto_tree_add_item(tree, hf_llrp_gpi_port, tvb,
                        offset, 2, ENC_BIG_ENDIAN);
            offset += 2;

            gpo_port = tvb_get_ntohs(tvb, offset);
            if (gpo_port == LLRP_GPO_PORT_ALL)
                gpo_item = proto_tree_add_uint_format(tree, hf_llrp_gpo_port, tvb,
                        offset, 2, gpo_port, "All GPO Ports (%u)", gpo_port);
            else
                gpo_item = proto_tree_add_item(tree, hf_llrp_gpo_port, tvb,
                        offset, 2, ENC_BIG_ENDIAN);
            offset += 2;

            switch (requested_data)
            {
                case LLRP_CONF_ALL:
                    break;
                case LLRP_CONF_ANTENNA_PROPERTIES:
                case LLRP_CONF_ANTENNA_CONFIGURATION:
                    /
                    proto_item_append_text(gpi_item, " (Ignored)");
                    proto_item_append_text(gpo_item, " (Ignored)");
                    break;
                case LLRP_CONF_IDENTIFICATION:
                case LLRP_CONF_RO_REPORT_SPEC:
                case LLRP_CONF_READER_EVENT_NOTIFICATION_SPEC:
                case LLRP_CONF_ACCESS_REPORT_SPEC:
                case LLRP_CONF_LLRP_CONFIGURATION_STATE:
                case LLRP_CONF_KEEPALIVE_SPEC:
                case LLRP_CONF_EVENTS_AND_REPORTS:
                    /
                    proto_item_append_text(antenna_item, " (Ignored)");
                    /
                    proto_item_append_text(gpi_item, " (Ignored)");
                    proto_item_append_text(gpo_item, " (Ignored)");
                    break;
                case LLRP_CONF_GPI_PORT_CURRENT_STATE:
                    /
                    proto_item_append_text(antenna_item, " (Ignored)");
                    /
                    proto_item_append_text(gpo_item, " (Ignored)");
                    break;
                case LLRP_CONF_GPO_WRITE_DATA:
                    /
                    proto_item_append_text(antenna_item, " (Ignored)");
                    /
                    proto_item_append_text(gpi_item, " (Ignored)");
                    break;
                default:
                    /
                    proto_item_append_text(antenna_item, " (Ignored)");
                    /
                    expert_add_info_format(pinfo, request_item, &ei_llrp_req_conf,
                            "Unrecognized configuration request: %u",
                            requested_data);
                    /
                    proto_item_append_text(gpi_item, " (Ignored)");
                    proto_item_append_text(gpo_item, " (Ignored)");
                    break;
            };
            ends_with_parameters = TRUE;
            break;
        /
        /
        case LLRP_TYPE_SET_READER_CONFIG:
            proto_tree_add_item(tree, hf_llrp_rest_fact, tvb, offset, 1, ENC_NA);
            offset++;
            ends_with_parameters = TRUE;
            break;
        case LLRP_TYPE_SET_PROTOCOL_VERSION:
            proto_tree_add_item(tree, hf_llrp_version, tvb, offset, 1, ENC_BIG_ENDIAN);
            break;
        case LLRP_TYPE_GET_SUPPORTED_VERSION_RESPONSE:
            proto_tree_add_item(tree, hf_llrp_cur_ver, tvb, offset, 1, ENC_BIG_ENDIAN);
            offset++;
            proto_tree_add_item(tree, hf_llrp_sup_ver, tvb, offset, 1, ENC_BIG_ENDIAN);
            offset++;
            ends_with_parameters = TRUE;
            break;
        case LLRP_TYPE_CUSTOM_MESSAGE:
            vendor = tvb_get_ntohl(tvb, offset);
            proto_tree_add_item(tree, hf_llrp_vendor, tvb, offset, 4, ENC_BIG_ENDIAN);
            offset += 4;
            /
            switch(vendor) {
            case LLRP_VENDOR_IMPINJ:
                dissect_custom_message = dissect_llrp_impinj_message;
                ends_with_parameters = TRUE;
                break;
            }
            if (dissect_custom_message)
                offset = dissect_custom_message(tvb, pinfo, tree, offset);
            break;
        /
        case LLRP_TYPE_KEEPALIVE:
        case LLRP_TYPE_KEEPALIVE_ACK:
        case LLRP_TYPE_CLOSE_CONNECTION:
        case LLRP_TYPE_GET_ROSPECS:
        case LLRP_TYPE_GET_SUPPORTED_VERSION:
            break;
        default:
            /
            DISSECTOR_ASSERT_NOT_REACHED();
    };
    if(ends_with_parameters) {
        offset = dissect_llrp_parameters(tvb, pinfo, tree, offset, tvb_reported_length(tvb));
    }
    if(tvb_reported_length_remaining(tvb, offset) != 0) {
        /
        expert_add_info_format(pinfo, tree, &ei_llrp_invalid_length,
                "Incorrect length of message: %u bytes decoded, but %u bytes available.",
                offset, tvb_reported_length(tvb));
    }
}
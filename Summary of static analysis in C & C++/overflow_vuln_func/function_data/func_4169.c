static int mv_read_header(AVFormatContext *avctx)
{
    MvContext *mv = avctx->priv_data;
    AVIOContext *pb = avctx->pb;
    AVStream *ast = NULL, *vst = NULL; //initialization to suppress warning
    int version, i;

    avio_skip(pb, 4);

    version = avio_rb16(pb);
    if (version == 2) {
        uint64_t timestamp;
        int v;
        avio_skip(pb, 22);

        /
        ast = avformat_new_stream(avctx, NULL);
        if (!ast)
            return AVERROR(ENOMEM);

        vst = avformat_new_stream(avctx, NULL);
        if (!vst)
            return AVERROR(ENOMEM);
        avpriv_set_pts_info(vst, 64, 1, 15);
        vst->codec->codec_type = AVMEDIA_TYPE_VIDEO;
        vst->avg_frame_rate    = av_inv_q(vst->time_base);
        vst->nb_frames         = avio_rb32(pb);
        v = avio_rb32(pb);
        switch (v) {
        case 1:
            vst->codec->codec_id = AV_CODEC_ID_MVC1;
            break;
        case 2:
            vst->codec->pix_fmt  = AV_PIX_FMT_ARGB;
            vst->codec->codec_id = AV_CODEC_ID_RAWVIDEO;
            break;
        default:
            avpriv_request_sample(avctx, "Video compression %i", v);
            break;
        }
        vst->codec->codec_tag = 0;
        vst->codec->width     = avio_rb32(pb);
        vst->codec->height    = avio_rb32(pb);
        avio_skip(pb, 12);

        ast->codec->codec_type  = AVMEDIA_TYPE_AUDIO;
        ast->nb_frames          = vst->nb_frames;
        ast->codec->sample_rate = avio_rb32(pb);
        avpriv_set_pts_info(ast, 33, 1, ast->codec->sample_rate);
        if (set_channels(avctx, ast, avio_rb32(pb)) < 0)
            return AVERROR_INVALIDDATA;

        v = avio_rb32(pb);
        if (v == AUDIO_FORMAT_SIGNED) {
            ast->codec->codec_id = AV_CODEC_ID_PCM_S16BE;
        } else {
            avpriv_request_sample(avctx, "Audio compression (format %i)", v);
        }

        avio_skip(pb, 12);
        var_read_metadata(avctx, "title", 0x80);
        var_read_metadata(avctx, "comment", 0x100);
        avio_skip(pb, 0x80);

        timestamp = 0;
        for (i = 0; i < vst->nb_frames; i++) {
            uint32_t pos   = avio_rb32(pb);
            uint32_t asize = avio_rb32(pb);
            uint32_t vsize = avio_rb32(pb);
            avio_skip(pb, 8);
            av_add_index_entry(ast, pos, timestamp, asize, 0, AVINDEX_KEYFRAME);
            av_add_index_entry(vst, pos + asize, i, vsize, 0, AVINDEX_KEYFRAME);
            timestamp += asize / (ast->codec->channels * 2);
        }
    } else if (!version && avio_rb16(pb) == 3) {
        avio_skip(pb, 4);

        read_table(avctx, NULL, parse_global_var);

        if (mv->nb_audio_tracks > 1) {
            avpriv_request_sample(avctx, "Multiple audio streams support");
            return AVERROR_PATCHWELCOME;
        } else if (mv->nb_audio_tracks) {
            ast = avformat_new_stream(avctx, NULL);
            if (!ast)
                return AVERROR(ENOMEM);
            ast->codec->codec_type = AVMEDIA_TYPE_AUDIO;
            read_table(avctx, ast, parse_audio_var);
            if (mv->acompression == 100 &&
                mv->aformat == AUDIO_FORMAT_SIGNED &&
                ast->codec->bits_per_coded_sample == 16) {
                ast->codec->codec_id = AV_CODEC_ID_PCM_S16BE;
            } else {
                avpriv_request_sample(avctx,
                                      "Audio compression %i (format %i, sr %i)",
                                      mv->acompression, mv->aformat,
                                      ast->codec->bits_per_coded_sample);
                ast->codec->codec_id = AV_CODEC_ID_NONE;
            }
            if (ast->codec->channels <= 0) {
                av_log(avctx, AV_LOG_ERROR, "No valid channel count found.\n");
                return AVERROR_INVALIDDATA;
            }
        }

        if (mv->nb_video_tracks > 1) {
            avpriv_request_sample(avctx, "Multiple video streams support");
            return AVERROR_PATCHWELCOME;
        } else if (mv->nb_video_tracks) {
            vst = avformat_new_stream(avctx, NULL);
            if (!vst)
                return AVERROR(ENOMEM);
            vst->codec->codec_type = AVMEDIA_TYPE_VIDEO;
            read_table(avctx, vst, parse_video_var);
        }

        if (mv->nb_audio_tracks)
            read_index(pb, ast);

        if (mv->nb_video_tracks)
            read_index(pb, vst);
    } else {
        avpriv_request_sample(avctx, "Version %i", version);
        return AVERROR_PATCHWELCOME;
    }

    return 0;
}
static int
dissect_lpp_T_cnavOMEGA0(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 726 "../../asn1/lpp/lpp.cnf"
  gint64 cnavOMEGA0;
  int curr_offset = offset;
  offset = dissect_per_constrained_integer_64b(tvb, offset, actx, tree, hf_index,
                                                            G_GINT64_CONSTANT(-4294967296), 4294967295U, &cnavOMEGA0, FALSE);




#line 731 "../../asn1/lpp/lpp.cnf"
  PROTO_ITEM_SET_HIDDEN(actx->created_item);
  actx->created_item = proto_tree_add_int64_format_value(tree, hf_index, tvb, curr_offset>>3, (offset+7-curr_offset)>>3,
                                                         cnavOMEGA0, "%e semi-circles (%"G_GINT64_MODIFIER"d)",
                                                         (double)cnavOMEGA0*pow(2, -32), cnavOMEGA0);


  return offset;
}
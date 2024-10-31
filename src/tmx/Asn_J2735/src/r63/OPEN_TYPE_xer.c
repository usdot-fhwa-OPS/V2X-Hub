/*
 * Copyright (c) 2017 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 * Redistribution and modifications are permitted subject to BSD license.
 */
#include <asn_internal.h>
#include <OPEN_TYPE.h>
#include <constr_CHOICE.h>

asn_dec_rval_t
OPEN_TYPE_xer_get(const asn_codec_ctx_t *opt_codec_ctx,
                  const asn_TYPE_descriptor_t *td, void *sptr,
                  const asn_TYPE_member_t *elm, const void *ptr, size_t size) {
    size_t consumed_myself = 0;
    asn_type_selector_result_t selected;
    void *memb_ptr;   /* Pointer to the member */
    void **memb_ptr2; /* Pointer to that pointer */
    void *inner_value;
    asn_dec_rval_t rv;

    int xer_context = 0;
    ssize_t ch_size;
    pxer_chunk_type_e ch_type;

    if(!(elm->flags & ATF_OPEN_TYPE)) {
        ASN__DECODE_FAILED;
    }

    if(!elm->type_selector) {
        ASN_DEBUG("Type selector is not defined for Open Type %s->%s->%s",
                  td->name, elm->name, elm->type->name);
        ASN__DECODE_FAILED;
    }

    selected = elm->type_selector(td, sptr);
    if(!selected.presence_index) {
        ASN__DECODE_FAILED;
    }

    /* Fetch the pointer to this member */
    assert(elm->flags == ATF_OPEN_TYPE);
    if(elm->flags & ATF_POINTER) {
        memb_ptr2 = (void **)((char *)sptr + elm->memb_offset);
    } else {
        memb_ptr = (char *)sptr + elm->memb_offset;
        memb_ptr2 = &memb_ptr;
    }
    if(*memb_ptr2 != NULL) {
        /* Make sure we reset the structure first before encoding */
        if(CHOICE_variant_set_presence(elm->type, *memb_ptr2, 0)
           != 0) {
            ASN__DECODE_FAILED;
        }
    }

    /*
     * Confirm wrapper.
     */
    for(;;) {
        ch_size = xer_next_token(&xer_context, ptr, size, &ch_type);
        if(ch_size < 0) {
            ASN__DECODE_FAILED;
        } else {
            switch(ch_type) {
            case PXER_WMORE:
                ASN__DECODE_STARVED;
            case PXER_COMMENT:
            case PXER_TEXT:
                ADVANCE(ch_size);
                continue;
            case PXER_TAG:
                break;
            }
            break;
        }
    }

    /*
     * Wrapper value confirmed.
     */
    switch(xer_check_tag(ptr, ch_size, elm->name)) {
    case XCT_OPENING:
        ADVANCE(ch_size);
        break;
    case XCT_BROKEN:
    default:
        ASN__DECODE_FAILED;
    }

    inner_value =
        (char *)*memb_ptr2
        + elm->type->elements[selected.presence_index - 1].memb_offset;

    rv = selected.type_descriptor->op->xer_decoder(
        opt_codec_ctx, selected.type_descriptor, &inner_value, NULL, ptr, size);
    ADVANCE(rv.consumed);
    rv.consumed = 0;
    switch(rv.code) {
    case RC_OK:
        if(CHOICE_variant_set_presence(elm->type, *memb_ptr2,
                                       selected.presence_index)
           == 0) {
            break;
        } else {
            rv.code = RC_FAIL;
        }
        /* Fall through */
    case RC_FAIL:
        /* Point to a best position where failure occurred */
        rv.consumed = consumed_myself;
        /* Fall through */
    case RC_WMORE:
        /* Wrt. rv.consumed==0:
         * In case a genuine RC_WMORE, the whole Open Type decoding
         * will have to be restarted.
         */
        if(*memb_ptr2) {
            if(elm->flags & ATF_POINTER) {
                ASN_STRUCT_FREE(*selected.type_descriptor, inner_value);
                *memb_ptr2 = NULL;
            } else {
                ASN_STRUCT_RESET(*selected.type_descriptor,
                                              inner_value);
            }
        }
        return rv;
    }

    /*
     * Finalize wrapper.
     */
    for(;;) {
        ch_size = xer_next_token(&xer_context, ptr, size, &ch_type);
        if(ch_size < 0) {
            ASN__DECODE_FAILED;
        } else {
            switch(ch_type) {
            case PXER_WMORE:
                ASN__DECODE_STARVED;
            case PXER_COMMENT:
            case PXER_TEXT:
                ADVANCE(ch_size);
                continue;
            case PXER_TAG:
                break;
            }
            break;
        }
    }

    /*
     * Wrapper value confirmed.
     */
    switch(xer_check_tag(ptr, ch_size, elm->name)) {
    case XCT_CLOSING:
        ADVANCE(ch_size);
        break;
    case XCT_BROKEN:
    default:
        ASN__DECODE_FAILED;
    }

    rv.consumed += consumed_myself;

    return rv;
}

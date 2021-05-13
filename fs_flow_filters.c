#include <stdint.h>
#include <rte_common.h>
#include <rte_flow.h>
#include <rte_ether.h>

#include "fs_flow_filters.h"

// References:
//            /dpdk/examples/flow_filtering
//            https://doc.dpdk.org/guides/sample_app_ug/flow_filtering.html#compiling-the-application
//            DPDK 20.11-rc1  Section 12    example code: Section9.3.4 

typedef struct simple_flow_def_struct {
      uint16_t port_id;       // port to apply filter to
      uint16_t rx_q;          // if a match, queue to place flow on.
      uint32_t src_mac;        // 
      uint32_t src_mac_mask;      // portion of src IP to use as part of filter
 

      uint32_t src_ip;        // 
      uint32_t src_ip_mask;      // portion of src IP to use as part of filter
      uint32_t dest_ip;
      uint32_t dest_ip_mask;
      uint16_t type;          // tcp, udp, icmp, ...  0 is don't care
      } fs_flow_def;



#define MAX_PATTERN_NUM         4
#define MAX_ACTION_NUM          2


struct rte_flow *
generate_ipv4_flow(uint16_t port_id, uint16_t rx_q,
                uint32_t src_ip, uint32_t src_mask,
                uint32_t dest_ip, uint32_t dest_mask,
                struct rte_flow_error *error);

/**
 * create a flow rule that sends packets with matching src and dest ip
 * to selected queue.
 *
 * @param port_id
 *   The selected port.
 * @param rx_q
 *   The selected target queue.
 * @param src_ip
 *   The src ip value to match the input packet.
 * @param src_mask
 *   The mask to apply to the src ip.
 * @param dest_ip
 *   The dest ip value to match the input packet.
 * @param dest_mask
 *   The mask to apply to the dest ip.
 * @param[out] error
 *   Perform verbose error reporting if not NULL.
 *
 * @return
 *   A flow if the rule could be created else return NULL.
 */

struct rte_flow *
generate_ipv4_flow(uint16_t port_id, uint16_t rx_q,
                uint32_t src_ip, uint32_t src_mask,
                uint32_t dest_ip, uint32_t dest_mask,
                struct rte_flow_error *error)
{
        struct rte_flow_attr attr;
        struct rte_flow_item pattern[MAX_PATTERN_NUM];
        struct rte_flow_action action[MAX_ACTION_NUM];
        struct rte_flow *flow = NULL;
#ifdef ACTION_QUEUE
        struct rte_flow_action_queue    queue = { .index = rx_q };
#else
        struct rte_flow_action_port_id  action_port_id = { .id = 0 };   // hard wire on port. 
#endif
        struct rte_flow_item_ipv4 ip_spec;
        struct rte_flow_item_ipv4 ip_mask;
        int res;

        printf("%d - rx_q\n",rx_q);

        memset(pattern, 0, sizeof(pattern));
        memset(action, 0, sizeof(action));

        /*
         * set the rule attribute.
         * in this case only ingress packets will be checked.
         */
        memset(&attr, 0, sizeof(struct rte_flow_attr));
        attr.ingress = 1;



#ifdef ACTION_QUEUE
        /*
         * create the action sequence.
         * one action only,  move packet to queue
         */
        action[0].type = RTE_FLOW_ACTION_TYPE_QUEUE;
        action[0].conf = &queue;
        action[1].type = RTE_FLOW_ACTION_TYPE_END;
#else
        action[0].type = RTE_FLOW_ACTION_TYPE_PORT_ID;
        action[0].conf = &action_port_id;
        action[1].type = RTE_FLOW_ACTION_TYPE_END;
#endif


        /*
         * set the first level of the pattern (ETH).
         * since in this example we just want to get the
         * ipv4 we set this level to allow all.
         */
        pattern[0].type = RTE_FLOW_ITEM_TYPE_ETH;

        /*
         * setting the second level of the pattern (IP).
         * in this example this is the level we care about
         * so we set it according to the parameters.
         */
        memset(&ip_spec, 0, sizeof(struct rte_flow_item_ipv4));
        memset(&ip_mask, 0, sizeof(struct rte_flow_item_ipv4));
        ip_spec.hdr.dst_addr = htonl(dest_ip);
        ip_mask.hdr.dst_addr = htonl(dest_mask);
        ip_spec.hdr.src_addr = htonl(src_ip);
        ip_mask.hdr.src_addr = htonl(src_mask);
        pattern[1].type = RTE_FLOW_ITEM_TYPE_IPV4;
        pattern[1].spec = &ip_spec;
        pattern[1].mask = &ip_mask;

        /* the final level must be always type end */
        pattern[2].type = RTE_FLOW_ITEM_TYPE_END;

        res = rte_flow_validate(port_id, &attr, pattern, action, error);
        if (!res)
                flow = rte_flow_create(port_id, &attr, pattern, action, error);

        return flow;
}



/*
 *
 *   same as above, but add 1 more check for udp and port number.
 *
 */

struct rte_flow *
generate_ipv4_udp_flow(uint16_t port_id, uint16_t rx_q,
                  uint32_t src_ip, uint32_t src_mask,
                 uint32_t dest_ip, uint32_t dest_mask,
                 uint16_t src_port, uint16_t src_port_mask,
                 uint16_t dest_port, uint16_t dest_port_mask,
                struct rte_flow_error *error)
{
        struct rte_flow_attr attr;
        struct rte_flow_item pattern[MAX_PATTERN_NUM];
        struct rte_flow_action action[MAX_ACTION_NUM];
        struct rte_flow *flow = NULL;
        struct rte_flow_action_queue queue = { .index = rx_q };
        struct rte_flow_item_ipv4 ip_spec;
        struct rte_flow_item_ipv4 ip_mask;
        struct rte_flow_item_udp  udp_spec;
        struct rte_flow_item_udp  udp_mask;
        int res;

        memset(pattern, 0, sizeof(pattern));
        memset(action, 0, sizeof(action));

        /*
         * set the rule attribute.
         * in this case only ingress packets will be checked.
         */
        memset(&attr, 0, sizeof(struct rte_flow_attr));
        attr.ingress = 1;

        /*
         * create the action sequence.
         * one action only,  move packet to queue
         */
        action[0].type = RTE_FLOW_ACTION_TYPE_QUEUE;
        action[0].conf = &queue;
        action[1].type = RTE_FLOW_ACTION_TYPE_END;

        /*
         * set the first level of the pattern (ETH).
         * since in this example we just want to get the
         * ipv4 we set this level to allow all.
         */
        pattern[0].type = RTE_FLOW_ITEM_TYPE_ETH;

        /*
         * setting the second level of the pattern (IP).
         * in this example this is the level we care about
         * so we set it according to the parameters.
         */
        memset(&ip_spec, 0, sizeof(struct rte_flow_item_ipv4));
        memset(&ip_mask, 0, sizeof(struct rte_flow_item_ipv4));
        ip_spec.hdr.dst_addr = htonl(dest_ip);
        ip_mask.hdr.dst_addr = htonl(dest_mask);
        ip_spec.hdr.src_addr = htonl(src_ip);
        ip_mask.hdr.src_addr = htonl(src_mask);
//        ip_spec.hdr.next_proto_id = 0x11;     // UDP = 17 (0x11)  TCP=6  IPSEC=50
//        ip_mask.hdr.next_proto_id = 0xff;     
        pattern[1].type = RTE_FLOW_ITEM_TYPE_IPV4;
        pattern[1].spec = &ip_spec;
        pattern[1].mask = &ip_mask;

        /*
         * setting the third level of the pattern (udp).
         * in this example this is the level we care about
         * so we set it according to the parameters.
         */
        memset(&udp_spec, 0, sizeof(struct rte_flow_item_udp));
        memset(&udp_mask, 0, sizeof(struct rte_flow_item_udp));
        udp_spec.hdr.src_port = htons(src_port);
        udp_mask.hdr.src_port = htons(src_port_mask);
        udp_spec.hdr.dst_port = htons(dest_port);
        udp_mask.hdr.dst_port = htons(dest_port_mask);
        pattern[2].type = RTE_FLOW_ITEM_TYPE_UDP;
        pattern[2].spec = &udp_spec;
        pattern[2].mask = &udp_mask;

        /* the final level must be always type end */
        pattern[3].type = RTE_FLOW_ITEM_TYPE_END;

        res = rte_flow_validate(port_id, &attr, pattern, action, error);
        if (!res)
                flow = rte_flow_create(port_id, &attr, pattern, action, error);

        return flow;
}


/**
 * create a flow rule that sends packets with matching ethtype
 * to selected queue.
 *
 * @param port_id
 *   The selected port.
 * @param rx_q
 *   The selected target queue.
 * @param src_mac
 *   The src mac value to match the input packet.
 * @param src_mac_mask
 *   The mask to apply to the src mac.
 * @param dst_mac
 *   The dest mac value to match the input packet.
 * @param dst_mac_mask
 *   The mask to apply to the dest mac.
 * @param ethtype
 *   ethtype field.
 * @param ethtype_mask
 *   The mask to apply to the ethtype.
 * @param[out] error
 *   Perform verbose error reporting if not NULL.
 *
 * @return
 *   A flow if the rule could be created else return NULL.
 */
struct rte_flow *
generate_ethtype_flow(uint16_t port_id, uint32_t port,
                struct rte_ether_addr *src_mac, struct rte_ether_addr *src_mac_mask,
                struct rte_ether_addr *dst_mac, struct rte_ether_addr *dst_mac_mask,
                rte_be16_t   ethtype , rte_be16_t  ethtype_mask , 
                struct rte_flow_error *error)
{
        struct rte_flow_attr attr;
        struct rte_flow_item pattern[MAX_PATTERN_NUM];
        struct rte_flow_action action[MAX_ACTION_NUM];
        struct rte_flow *flow = NULL;
        struct rte_flow_action_port_id  action_port_id = { .id = port };
        struct rte_flow_item_eth    mac_spec;
        struct rte_flow_item_eth    mac_mask;
        int res;

        memset(pattern, 0, sizeof(pattern));
        memset(action, 0, sizeof(action));

        /*
         * set the rule attribute.
         * in this case only ingress packets will be checked.
         */
        memset(&attr, 0, sizeof(struct rte_flow_attr));
        attr.ingress = 1;

        /*
         * create the action sequence.
         * one action only,  move packet to queue
         */
        action[0].type = RTE_FLOW_ACTION_TYPE_PORT_ID;
        action[0].conf = &action_port_id;
        action[1].type = RTE_FLOW_ACTION_TYPE_END;

        /*
         * set the first level of the pattern (ETH).
         * in this example this is the level we care about
         * so we set it according to the parameters.
         */
        memset(&mac_spec, 0, sizeof(struct rte_flow_item_eth));
        memset(&mac_mask, 0, sizeof(struct rte_flow_item_eth));

        memcpy (&mac_spec.dst,dst_mac,6);
        memcpy (&mac_mask.dst,dst_mac_mask,6);
        memcpy (&mac_spec.src,src_mac,6);
        memcpy (&mac_mask.src,src_mac_mask,6);

        mac_spec.type = ethtype;
        mac_mask.type = ethtype_mask;
        pattern[0].type = RTE_FLOW_ITEM_TYPE_ETH;
        pattern[0].spec = &mac_spec;
        pattern[0].mask = &mac_mask;

        /* the final level must be always type end */
        pattern[1].type = RTE_FLOW_ITEM_TYPE_END;

        res = rte_flow_validate(port_id, &attr, pattern, action, error);
        if (!res)
                flow = rte_flow_create(port_id, &attr, pattern, action, error);

        return flow;
}




void print_rte_flow_error(const char * string, struct rte_flow_error *p)
{
     printf(" %s rte_flow_error:\n",string);
     printf("    type:     %d \n",p->type);
     printf("    cause:    0x%p\n",p->cause);
     printf("    message:  %s\n",p->message); 
}


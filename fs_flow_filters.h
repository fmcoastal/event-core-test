#ifndef _fs_flow_filter_h
#define _fs_flow_filter_h


struct rte_flow *
generate_ipv4_flow(uint16_t port_id, uint16_t rx_q,
                uint32_t src_ip, uint32_t src_mask,
                uint32_t dest_ip, uint32_t dest_mask,
                struct rte_flow_error *error);


struct rte_flow *
generate_ipv4_udp_flow(uint16_t port_id, uint16_t rx_q,
                  uint32_t src_ip, uint32_t src_mask,
                 uint32_t dest_ip, uint32_t dest_mask,
                 uint16_t src_port, uint16_t src_port_mask,
                 uint16_t dest_port, uint16_t dest_port_mask,
                struct rte_flow_error *error);


struct rte_flow *
generate_ethtype_flow(uint16_t port_id, uint32_t port,
                struct rte_ether_addr *src_mac, struct rte_ether_addr *src_mac_mask,
                struct rte_ether_addr *dst_mac, struct rte_ether_addr *dst_mac_mask,
                rte_be16_t   ethtype , rte_be16_t  ethtype_mask ,
                struct rte_flow_error *error);






void print_rte_flow_error(const char * string, struct rte_flow_error *p);


#endif

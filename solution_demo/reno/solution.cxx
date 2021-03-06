#include "solution.hxx"
#include <algorithm>


void SolutionInit()
{
    your_parameter["max_packet_size"] = 1350;
    your_parameter["init_ssthresh"] = 1 * your_parameter["max_packet_size"];
    your_parameter["last_time"] = 0;
    your_parameter["ssthresh"] = your_parameter["init_ssthresh"];
    // 0, 1, 2 -> ["slow_start", "congestion_avoidance", "fast_recovery"]
    your_parameter["cur_state"] = 0;
    your_parameter["ack_nums"] = 0;
}

uint64_t SolutionSelectBlock(Block* blocks, uint64_t block_num, uint64_t current_time)
{
    /************** START CODE HERE ***************/
    // return the id of block you want to send, for example:
    if (your_parameter.count("last_time") > 0)
    {
        uint64_t last_time = your_parameter["last_time"];
        fprintf(stderr, "last_time = %lu, current_time = %lu\n", last_time, current_time);
    }

    your_parameter["last_time"] = current_time;
    uint64_t best_block_index = -1;
    for (int i = 0; i < block_num; i++)
    {
        if (best_block_index == -1)
            best_block_index = i;
        else
        {
            double best_block_create_time = blocks[best_block_index].block_create_time;
            double packet_block_create_time = blocks[i].block_create_time;

            if (current_time - best_block_create_time >= blocks[best_block_index].block_deadline ||
                best_block_create_time > packet_block_create_time ||
                (current_time - best_block_create_time) * blocks[best_block_index].block_deadline >
                    (current_time - packet_block_create_time) * blocks[i].block_deadline)
                best_block_index = i;
        }
    }
    return blocks[best_block_index].block_id;
    /************** END CODE HERE ***************/
}

void SolutionCcTrigger(CcInfo* cc_infos, uint64_t cc_num, uint64_t *congestion_window, uint64_t *pacing_rate)
{
    /************** START CODE HERE ***************/
    uint64_t cwnd = *congestion_window;
    for (uint64_t i = 0; i < cc_num; i++)
    {
        char event_type = cc_infos[i].event_type;
        // fprintf(stderr, "event_type=%s\n", event_type);
        const uint64_t max_packet_size = 1350;
        const uint64_t init_ssthresh = 2 * max_packet_size;
        if (your_parameter.count("ssthresh") <= 0)
            your_parameter["ssthresh"] = init_ssthresh;
        // return new cwnd, for example:
        uint64_t ssthresh = your_parameter["ssthresh"];

        if (event_type == 'D')
        { // EVENT_TYPE_DROP
            your_parameter["cur_state"] = 2;
            your_parameter["ack_nums"] = 0;
        }

        if (event_type == 'F')
        { // EVENT_TYPE_FINISHED
            your_parameter["ack_nums"] += max_packet_size;

            if (your_parameter["cur_state"] == 0)
            {
                if (your_parameter["ack_nums"] >= cwnd)
                {
                    cwnd = cwnd * 2;
                    your_parameter["ack_nums"] = 0;
                }
                if (cwnd >= your_parameter["ssthresh"])
                {
                    your_parameter["cur_state"] = 1;
                }
            }
            else if (your_parameter["cur_state"] == 1)
            {
                if (your_parameter["ack_nums"] >= cwnd)
                {
                    cwnd += max_packet_size;
                    your_parameter["ack_nums"] = 0;
                }
            }
        }

        if (your_parameter["cur_state"] == 2)
        {
            your_parameter["ssthresh"] = max((uint64_t)(cwnd / 2), max_packet_size);
            cwnd = your_parameter["ssthresh"];
            your_parameter["cur_state"] = 1;
        }

        // fprintf(stderr,"new cwnd: %lu, ssthresh = %lu\n", cwnd, your_parameter["ssthresh"]);
    }
    // fprintf(stderr,"new cwnd: %lu, ssthresh = %lu\n", cwnd, your_parameter["ssthresh"]);
    *pacing_rate = 123456789;
    *congestion_window = cwnd;
    /************** END CODE HERE ***************/
}
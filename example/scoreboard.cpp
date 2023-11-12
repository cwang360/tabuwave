/**
 * Author:          Cynthia Wang
 * Date created:    11/10/2023
 * Organization:    ECE 4122
 *
 * Description:
 * Driver code using Verilator to simulate updates in the scoreboard module
 * and dump the waveform data into waveform.vcd. Its purpose is
 * solely to generate a VCD file that is used to show a use case
 * of tabuwave.
*/

#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "obj_dir/Vscoreboard.h" 

vluint64_t sim_time = 0;
int total_errors = 0;

/**
 * @brief Updates the simulator's time and the waveform trace
 * 
 * @param dut (Vscoreboard*) pointer to an instance of the verilated model
 * @param m_trace (VerilatedVcdC*) pointer to the VerilatedVcdC trace object
 */
void update_sim(Vscoreboard *dut, VerilatedVcdC *m_trace)
{
    dut->eval();
    m_trace->dump(sim_time);
    sim_time++;
}

/**
 * @brief Toggles clock and updates the sim time/trace
 * 
 * @param dut (Vscoreboard*) - pointer to an instance of the verilated model
 * @param m_trace (VerilatedVcdC*) - pointer to the VerilatedVcdC trace object
 */
void toggle_pos_edge_clk(Vscoreboard *dut, VerilatedVcdC *m_trace)
{
    dut->clk = 0;
    update_sim(dut, m_trace);
    dut->clk = 1;
    update_sim(dut, m_trace);
}

/**
 * @brief Updates the scoreboard. Fake updating multiple indices at a time by 
 * calling dut->eval() without incrementing sim time or calling m_trace->dump
 * 
 * @param dut (Vscoreboard*) - pointer to an instance of the verilated model
 * @param m_trace (VerilatedVcdC*) - pointer to the VerilatedVcdC trace object
 * @param in_idx (int) - index of scoreboard to update
 * @param in_pc_vld (int) - true if in_pc is valid
 * @param in_pc (int) - pc of entry
 * @param in_opcode_vld (int) - true if in_opcode is valid
 * @param in_opcode (int) - opcode of entry
 * @param in_completed_vld (int) - 1 to mark entry as completed, otherwise 0
 * @param in_invalidate_vld (int) - 1 to invalidate entry, otherwise 0
 */
void update_scrbrd(
    Vscoreboard *dut, VerilatedVcdC *m_trace, 
    int in_idx, 
    int in_pc_vld, 
    int in_pc, 
    int in_opcode_vld, 
    int in_opcode, 
    int in_completed_vld, 
    int in_invalidate_vld)
{
    dut->in_idx = in_idx;
    dut->in_pc_vld = in_pc_vld;
    dut->in_pc = in_pc;
    dut->in_opcode_vld = in_opcode_vld;
    dut->in_opcode = in_opcode;
    dut->in_completed_vld = in_completed_vld;
    dut->in_invalidate_vld = in_invalidate_vld;
    dut->clk = 0;
    dut->eval();
    dut->clk = 1;
    dut->eval();
}

/**
 * @brief Main driver to initialize the module/trace, trigger reset, and simulate
 * updating the scoreboard.
 * 
 * @return 0 on success 
 */
int main()
{
    srand(time(NULL));
    // initialize the top module
    Vscoreboard *dut = new Vscoreboard; 
    Verilated::traceEverOn(true);
    VerilatedVcdC *m_trace = new VerilatedVcdC;
    
    dut->trace(m_trace, 5);
    m_trace->open("waveform.vcd");
    dut->clk = 0;
    update_sim(dut, m_trace);

    // reset
    dut->rst = 1;
    toggle_pos_edge_clk(dut, m_trace);
    dut->rst = 0;
    toggle_pos_edge_clk(dut, m_trace);

    // add data
    update_scrbrd(dut, m_trace, 0, 1, 0x2EEF0, 1, 0x33, 0, 0);
    update_scrbrd(dut, m_trace, 1, 1, 0x2EEF4, 1, 0x33, 0, 0);
    toggle_pos_edge_clk(dut, m_trace);

    update_scrbrd(dut, m_trace, 2, 1, 0x2EEF8, 1, 0x3, 0, 0);
    update_scrbrd(dut, m_trace, 3, 1, 0x2EEFC, 1, 0x3, 0, 0);
    update_scrbrd(dut, m_trace, 4, 1, 0x2EF00, 1, 0x33, 0, 0);
    toggle_pos_edge_clk(dut, m_trace);

    update_scrbrd(dut, m_trace, 5, 1, 0x2EF04, 1, 0x23, 0, 0);
    update_scrbrd(dut, m_trace, 6, 1, 0x2EF08, 1, 0x63, 0, 0);
    update_scrbrd(dut, m_trace, 0, 0, 0, 0, 0, 1, 1);
    update_scrbrd(dut, m_trace, 1, 0, 0, 0, 0, 1, 1);
    toggle_pos_edge_clk(dut, m_trace);

    update_scrbrd(dut, m_trace, 5, 1, 0x2EF04, 1, 0x23, 0, 0);
    update_scrbrd(dut, m_trace, 6, 1, 0x2EF08, 1, 0x63, 0, 0);
    toggle_pos_edge_clk(dut, m_trace);

    update_scrbrd(dut, m_trace, 7, 1, 0x2EF0C, 1, 0x23, 0, 0);
    update_scrbrd(dut, m_trace, 8, 1, 0x2EF10, 1, 0x63, 0, 0);
    update_scrbrd(dut, m_trace, 9, 1, 0x2EF14, 1, 0x23, 0, 0);
    update_scrbrd(dut, m_trace, 10, 1, 0x2EF18, 1, 0x63, 0, 0);
    toggle_pos_edge_clk(dut, m_trace);

    update_scrbrd(dut, m_trace, 11, 1, 0xBEEF8, 1, 0x3, 0, 0);
    update_scrbrd(dut, m_trace, 12, 1, 0xBEEFC, 1, 0x3, 0, 0);
    update_scrbrd(dut, m_trace, 13, 1, 0xBEF00, 1, 0x3, 0, 0);
    update_scrbrd(dut, m_trace, 14, 1, 0xBEF04, 1, 0x3, 0, 0);
    update_scrbrd(dut, m_trace, 6, 0, 0, 0, 0, 1, 0);
    update_scrbrd(dut, m_trace, 7, 0, 0, 0, 0, 0, 1);
    update_scrbrd(dut, m_trace, 8, 0, 0, 0, 0, 0, 1);
    update_scrbrd(dut, m_trace, 9, 0, 0, 0, 0, 0, 1);
    update_scrbrd(dut, m_trace, 10, 0, 0, 0, 0, 0, 1);
    toggle_pos_edge_clk(dut, m_trace);

    update_scrbrd(dut, m_trace, 15, 1, 0xBEF08, 1, 0x3, 0, 0);
    update_scrbrd(dut, m_trace, 16, 1, 0xBEF0C, 1, 0x3, 0, 0);
    update_scrbrd(dut, m_trace, 17, 1, 0xBEF10, 1, 0x3, 0, 0);
    update_scrbrd(dut, m_trace, 18, 1, 0xBEF14, 1, 0x3, 0, 0);
    toggle_pos_edge_clk(dut, m_trace);

    update_scrbrd(dut, m_trace, 2, 0, 0, 0, 0, 1, 1);
    update_scrbrd(dut, m_trace, 3, 0, 0, 0, 0, 1, 1);
    update_scrbrd(dut, m_trace, 11, 0, 0, 0, 0, 1, 0);
    update_scrbrd(dut, m_trace, 12, 0, 0, 0, 0, 1, 0);
    update_scrbrd(dut, m_trace, 13, 0, 0, 0, 0, 1, 0);
    update_scrbrd(dut, m_trace, 14, 0, 0, 0, 0, 1, 0);
    toggle_pos_edge_clk(dut, m_trace);

    update_scrbrd(dut, m_trace, 4, 0, 0, 0, 0, 1, 1);
    update_scrbrd(dut, m_trace, 5, 0, 0, 0, 0, 0, 1);
    update_scrbrd(dut, m_trace, 6, 0, 0, 0, 0, 0, 1);
    update_scrbrd(dut, m_trace, 11, 0, 0, 0, 0, 0, 1);
    update_scrbrd(dut, m_trace, 15, 0, 0, 0, 0, 1, 0);
    update_scrbrd(dut, m_trace, 16, 0, 0, 0, 0, 1, 0);
    update_scrbrd(dut, m_trace, 17, 0, 0, 0, 0, 1, 0);
    update_scrbrd(dut, m_trace, 18, 0, 0, 0, 0, 1, 0);
    toggle_pos_edge_clk(dut, m_trace);

    update_scrbrd(dut, m_trace, 12, 0, 0, 0, 0, 0, 1);
    update_scrbrd(dut, m_trace, 13, 0, 0, 0, 0, 0, 1);
    update_scrbrd(dut, m_trace, 14, 0, 0, 0, 0, 0, 1);
    update_scrbrd(dut, m_trace, 15, 0, 0, 0, 0, 0, 1);
    toggle_pos_edge_clk(dut, m_trace);

    update_scrbrd(dut, m_trace, 16, 0, 0, 0, 0, 0, 1);
    update_scrbrd(dut, m_trace, 17, 0, 0, 0, 0, 0, 1);
    update_scrbrd(dut, m_trace, 18, 0, 0, 0, 0, 0, 1);
    toggle_pos_edge_clk(dut, m_trace);

    m_trace->close();
    delete dut;

    std::cout << "Completed" << std::endl;

    exit(EXIT_SUCCESS);
}
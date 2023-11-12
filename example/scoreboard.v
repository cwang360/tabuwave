/*
 * Author:          Cynthia Wang
 * Date created:    11/10/2023
 * Organization:    ECE 4122
 *
 * Description:
 * A toy Verilog scoreboard that keeps track of PC, opcode, completed for
 * instructions.
*/

`timescale 1ns / 1ps

// Top module 
module scoreboard #(
    parameter SCRBRD_SIZE = 32,
    parameter PC_WIDTH = 32,
    parameter OPCODE_WIDTH = 7
) (
    input       clk,  // Clock
    input       rst,  // Reset 

    // Input data signals
    input [$clog2(SCRBRD_SIZE)-1:0] in_idx,

    input                      in_pc_vld,   
    input [PC_WIDTH-1:0]       in_pc,

    input                      in_opcode_vld,   
    input [OPCODE_WIDTH-1:0]   in_opcode,

    input                      in_completed_vld,   

    input                      in_invalidate_vld
    // Output signals (none)
);    
    /* verilator lint_off UNUSEDSIGNAL */
    reg  [SCRBRD_SIZE-1:0] scrbrd_vld;
    reg  [SCRBRD_SIZE-1:0] scrbrd_completed;

    reg  [PC_WIDTH-1:0]     scrbrd_pc [SCRBRD_SIZE-1:0];
    reg  [OPCODE_WIDTH-1:0] scrbrd_opcode [SCRBRD_SIZE-1:0];

    always @(posedge clk) begin
        if (rst) begin
            scrbrd_vld         <= 'b0;
        end 
        else begin
            if (in_invalidate_vld) begin 
                scrbrd_vld[in_idx] <= 1'b0;
            end
            if (in_completed_vld) begin 
                scrbrd_completed[in_idx] <= 1'b1;
            end
            if (in_pc_vld) begin 
                scrbrd_pc[in_idx] <= in_pc;
                scrbrd_vld[in_idx] <= 1'b1;
            end
            if (in_opcode_vld) begin 
                scrbrd_opcode[in_idx] <= in_opcode;
            end
        end
    end

endmodule: scoreboard

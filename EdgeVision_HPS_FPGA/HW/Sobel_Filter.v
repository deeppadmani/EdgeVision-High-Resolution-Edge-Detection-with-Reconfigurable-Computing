//======================================================================
// Project Name: Sobel Filter Implementation on FPGA
// Module Name: ghrd_top
// Description: 
// This module implements a 3x3 Sobel filter for edge detection in images.
// It processes 8-bit grayscale pixel inputs and outputs edge-detected 
// results. The module interfaces with an HPS (Hard Processor System) 
// for external communication and control.
//
// Key Features:
// - Sobel Filter Operation:
//     - Computes horizontal (Gx) and vertical (Gy) gradients
//     - Combines gradients to calculate edge magnitude
// - Interfaces with external DDR3 memory via HPS for storing and retrieving data
// - Uses internal line buffers for pixel storage and convolution operations
//
// Inputs:
// - rst (Reset): System reset signal
// - CLOCK_50: Clock signal at 50 MHz
// - input_row: 32-bit input containing three 8-bit pixel values
//
// Outputs:
// - output_row: 8-bit pixel value after Sobel processing
// - DRAM_* and HPS_DDR3_*: External memory and HPS interface signals
//
// Created By: Saumya Shah  &  Deep Padmani
// Date: 12/11/2024
//======================================================================

module Sobel_Filter(

		//////////RESET//////////
		input 				 rst,
		
		/////////OUTPUT//////////
		output reg [7:0] output_row,
		
      ///////// CLOCK /////////
      input              CLOCK_50,

      ///////// DRAM /////////
      output      [12:0] DRAM_ADDR,
      output      [1:0]  DRAM_BA,
      output             DRAM_CAS_N,
      output             DRAM_CKE,
      output             DRAM_CLK,
      output             DRAM_CS_N,
      inout       [15:0] DRAM_DQ,
      output             DRAM_LDQM,
      output             DRAM_RAS_N,
      output             DRAM_UDQM,
      output             DRAM_WE_N,

      ///////// HPS /////////
      inout              HPS_CONV_USB_N,
      output      [14:0] HPS_DDR3_ADDR,
      output      [2:0]  HPS_DDR3_BA,
      output             HPS_DDR3_CAS_N,
      output             HPS_DDR3_CKE,
      output             HPS_DDR3_CK_N,
      output             HPS_DDR3_CK_P,
      output             HPS_DDR3_CS_N,
      output      [3:0]  HPS_DDR3_DM,
      inout       [31:0] HPS_DDR3_DQ,
      inout       [3:0]  HPS_DDR3_DQS_N,
      inout       [3:0]  HPS_DDR3_DQS_P,
      output             HPS_DDR3_ODT,
      output             HPS_DDR3_RAS_N,
      output             HPS_DDR3_RESET_N,
      input              HPS_DDR3_RZQ,
      output             HPS_DDR3_WE_N


);


////////// EXPORTS TO THE HPS-FPGA Communication//////////

wire [31:0] input_row;

////////// Internal Wires and Registers //////////
wire [3:0] fpga_debounced_buttons;
wire [9:0] fpga_led_internal;
wire hps_fpga_reset_n;
wire [2:0] hps_reset_req;
wire hps_cold_reset;
wire hps_warm_reset;
wire hps_debug_reset;
wire [27:0] stm_hw_events;

// Line buffer memory (3x3 window)
reg [7:0] line_buffer [0:2][0:2];  

// Sobel filter kernels (fixed 3x3 matrices)
wire signed [3:0] Gx [0:2][0:2];
wire signed [3:0] Gy [0:2][0:2];

// Internal signals for processing the Sobel filter

reg signed [15:0] sumX, sumY;
reg [7:0] absX, absY;
reg [8:0] magnitude;

integer i, j;

// Initialize Sobel kernels using assign statements (direct initialization)
    // Assign Sobel kernels
    assign Gx[0][0] = 1;  assign Gx[0][1] = 0;  assign Gx[0][2] = -1;
    assign Gx[1][0] = 2;  assign Gx[1][1] = 0;  assign Gx[1][2] = -2;
    assign Gx[2][0] = 1;  assign Gx[2][1] = 0;  assign Gx[2][2] = -1;
    
    assign Gy[0][0] = 1;  assign Gy[0][1] = 2;  assign Gy[0][2] = 1;
    assign Gy[1][0] = 0;  assign Gy[1][1] = 0;  assign Gy[1][2] = 0;
    assign Gy[2][0] = -1; assign Gy[2][1] = -2; assign Gy[2][2] = -1;
	 
initial begin
    output_row = 0;
    // Initialize line buffer
    for (i = 0; i < 3; i = i + 1)
        for (j = 0; j < 3; j = j + 1)
            line_buffer[i][j] = 0;
end

always @(posedge CLOCK_50) begin
 // Reset line buffer and output
    if (!rst) begin
        output_row <= 0;
        for (i = 0; i < 3; i = i + 1)
            for (j = 0; j < 3; j = j + 1)
                line_buffer[i][j] <= 0;
    end 
    else begin
///////////////////////// Shift pixels in line buffer horizontally/////////////////////////////////////////////////
        for (i = 0; i < 3; i = i + 1) begin
            line_buffer[i][0] <= line_buffer[i][1];
            line_buffer[i][1] <= line_buffer[i][2];
        end
        
///////////////////////// Load new pixel values in the line buffer////////////////////////////////////////////////
        line_buffer[0][2] <= input_row[23:16];
        line_buffer[1][2] <= input_row[15:8];
        line_buffer[2][2] <= input_row[7:0];

        
//////////////////////// Apply and Compute gradients using sobel filter/////////////////////////////////////////////////////

        sumX = 0;
        sumY = 0;
		  
		  for (i = 0; i < 3; i = i + 1) begin
            for (j = 0; j < 3; j = j + 1) begin
                sumX = sumX + $signed({1'b0, line_buffer[i][j]}) * Gx[i][j];
                sumY = sumY + $signed({1'b0, line_buffer[i][j]}) * Gy[i][j];
            end
        end

        // Calculate magnitude of edge gradient
        absX = (sumX < 0) ? -sumX[7:0] : sumX[7:0];
        absY = (sumY < 0) ? -sumY[7:0] : sumY[7:0];
        magnitude = absX + absY;

        // Threshold and invert the result
        output_row <= (8'hFF - magnitude[7:0]);
    end
end

// External system instantiation (HPS and DDR3 memory interface)
    soc_system u0 (

        .memory_mem_a                          ( HPS_DDR3_ADDR),                       //          memory.mem_a
        .memory_mem_ba                         ( HPS_DDR3_BA),                         //          .mem_ba
        .memory_mem_ck                         ( HPS_DDR3_CK_P),                       //          .mem_ck
        .memory_mem_ck_n                       ( HPS_DDR3_CK_N),                       //          .mem_ck_n
        .memory_mem_cke                        ( HPS_DDR3_CKE),                        //          .mem_cke
        .memory_mem_cs_n                       ( HPS_DDR3_CS_N),                       //          .mem_cs_n
        .memory_mem_ras_n                      ( HPS_DDR3_RAS_N),                      //          .mem_ras_n
        .memory_mem_cas_n                      ( HPS_DDR3_CAS_N),                      //          .mem_cas_n
        .memory_mem_we_n                       ( HPS_DDR3_WE_N),                       //          .mem_we_n
        .memory_mem_reset_n                    ( HPS_DDR3_RESET_N),                    //          .mem_reset_n
        .memory_mem_dq                         ( HPS_DDR3_DQ),                         //          .mem_dq
        .memory_mem_dqs                        ( HPS_DDR3_DQS_P),                      //          .mem_dqs
        .memory_mem_dqs_n                      ( HPS_DDR3_DQS_N),                      //          .mem_dqs_n
        .memory_mem_odt                        ( HPS_DDR3_ODT),                        //          .mem_odt
        .memory_oct_rzqin                      ( HPS_DDR3_RZQ),                        //          .oct_rzqin
		  .hps_0_f2h_stm_hw_events_stm_hwevents  (stm_hw_events),   							//        	hps_0_f2h_stm_hw_events.stm_hwevents
        .clk_clk                               (CLOCK_50),                             //          clk.clk	  
		  .hps_0_h2f_reset_reset_n               (hps_fpga_reset_n),               		//          hps_0_h2f_reset.reset_n
        .hps_0_f2h_warm_reset_req_reset_n      (~hps_warm_reset),      						//       	hps_0_f2h_warm_reset_req.reset_n
        .hps_0_f2h_debug_reset_req_reset_n     (~hps_debug_reset),     						//      		hps_0_f2h_debug_reset_req.reset_n
        .hps_0_f2h_cold_reset_req_reset_n      (~hps_cold_reset),       					//       	hps_0_f2h_cold_reset_req.reset_n
		  .pixel_in_pio_external_connection_export  (input_row),  								//  			pixel_in_pio_external_connection.export
        .pixel_out_pio_external_connection_export (output_row)  								// 			pixel_out_pio_external_connection.export
    );
	 
//////////////////////////// Reset management for HPS system/////////////////////////////////////////////////
hps_reset hps_reset_inst (
  .source_clk (CLOCK_50),
  .source     (hps_reset_req)
);

altera_edge_detector pulse_cold_reset (
  .clk       (CLOCK_50),
  .rst_n     (hps_fpga_reset_n),
  .signal_in (hps_reset_req[0]),
  .pulse_out (hps_cold_reset)
);
  defparam pulse_cold_reset.PULSE_EXT = 6;
  defparam pulse_cold_reset.EDGE_TYPE = 1;
  defparam pulse_cold_reset.IGNORE_RST_WHILE_BUSY = 1;

altera_edge_detector pulse_warm_reset (
  .clk       (CLOCK_50),
  .rst_n     (hps_fpga_reset_n),
  .signal_in (hps_reset_req[1]),
  .pulse_out (hps_warm_reset)
);
  defparam pulse_warm_reset.PULSE_EXT = 2;
  defparam pulse_warm_reset.EDGE_TYPE = 1;
  defparam pulse_warm_reset.IGNORE_RST_WHILE_BUSY = 1;
  
altera_edge_detector pulse_debug_reset (
  .clk       (CLOCK_50),
  .rst_n     (hps_fpga_reset_n),
  .signal_in (hps_reset_req[2]),
  .pulse_out (hps_debug_reset)
);
  defparam pulse_debug_reset.PULSE_EXT = 32;
  defparam pulse_debug_reset.EDGE_TYPE = 1;
  defparam pulse_debug_reset.IGNORE_RST_WHILE_BUSY = 1;

endmodule


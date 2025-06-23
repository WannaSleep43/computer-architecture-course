module stack_behaviour_normal(
    inout wire[3:0] IO_DATA, 
    input wire RESET, 
    input wire CLK, 
    input wire[1:0] COMMAND,
    input wire[2:0] INDEX
    ); 

    reg[3:0] stack[4:0];
    reg[2:0] last;

    integer rev = 3'b0;
    reg rw = 0;
    reg[3:0] out;
    assign IO_DATA = (rw) ? out : 4'bZZZZ;

    always @(RESET) begin
        for (integer i = 0; i < 5; i++) begin
            stack[i] = 5'b0;
        end
        last = 3'b0;
        rw = 0;
    end

    always @(posedge CLK) begin
        case (COMMAND)
            2'b00: begin rw = 0; end
            2'b01: begin 
                stack[last] = IO_DATA;
                rw = 0;
                last = (last == 4) ? 0 : last + 1;
            end
            2'b10: begin
                last = (last == 0) ? 4 : last - 1;
                rw = 1;
                out = stack[last];
            end
            2'b11: begin
                rev = (last + 5 - ((INDEX + 1) % 5)) % 5;
                rw = 1;
                out = stack[rev];
            end
        endcase
    end
endmodule

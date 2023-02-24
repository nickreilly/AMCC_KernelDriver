library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_unsigned.all;
use ieee.numeric_std.all;

--library SYNOPSYS;
--use SYNOPSYS.STD_LOGIC_arith.all;
--use SYNOPSYS.STD_LOGIC_unsigned.all;

entity DSP_u9 is
    port (
		CLK     : in STD_LOGIC; -- GCLK on chip. From DSP; (100 MHz?)
		NRESET   : in STD_LOGIC; -- GCLR on chip. From max 811
		NRD	: in STD_LOGIC; -- OE2/GCLK2 on chip. From DSP
		NWR	: in STD_LOGIC; -- OE1 on chip. From DSP
		SEQ	: buffer STD_LOGIC_VECTOR (19 downto 0); -- outs to pattern gen.
		D	: inout STD_LOGIC_VECTOR (19 downto 0); -- data bus from dsp.
		A	: in STD_LOGIC_VECTOR (5 downto 0); -- low order address of dsp
		NSS1	: in STD_LOGIC; -- slot select strobe from EPLD_B (U17.)
		NAA3	: in STD_LOGIC; -- Aux Addr 3 from DSP = Backplane access.
		BA	: out STD_LOGIC_VECTOR (5 downto 0); --buffered address to backplane
		NBRD	: out STD_LOGIC ; -- backplane read 
		NBWR	: out STD_LOGIC ; -- backplane write
		BRW	: out STD_LOGIC;  -- backplane ?? r/w??
		-- extend the shift register.
		SHIFTEN : in STD_LOGIC;
		SHIFTOUT : out STD_LOGIC;
		writedlyd : buffer STD_LOGIC
    	 );
end DSP_u9;

architecture U9_arch of DSP_u9 is

-- Internal Signals
signal CS : STD_LOGIC;
signal read : STD_LOGIC;
signal write : STD_LOGIC;
signal j6reg : STD_LOGIC_VECTOR(35 downto 0);
signal j6state : STD_LOGIC_VECTOR(1 downto 0);

constant SEQREGADD: STD_LOGIC_VECTOR:="001000"; -- write the source data directly to SEQ
constant SEQORADD: STD_LOGIC_VECTOR:="001001"; -- set bits in SEQ that are set in source.
constant SEQNANDADD: STD_LOGIC_VECTOR:="001010"; -- clear bits in SEQ that are set in source.
constant SEQTOGADD: STD_LOGIC_VECTOR:="001011"; -- toggle bits in SEQ that are set in source.
constant SEQSTATADD: STD_LOGIC_VECTOR:="001100"; -- write Frame status bits to sequence.
constant SEQFRMADD: STD_LOGIC_VECTOR:="001101"; -- write only frame clocks to the current sequence.
constant SEQROWADD: STD_LOGIC_VECTOR:="001110"; -- write only row clocks to the current sequence.
constant SEQCOLADD: STD_LOGIC_VECTOR:="001111"; -- write only column clocks to the current sequence.
constant J6DAT: STD_LOGIC_VECTOR:="0101"; -- write to J6DAT 3D4, 3D5, 3D6, 3D7
constant J6DATADD1: STD_LOGIC_VECTOR:="010100"; -- write to J6DAT. 3D4
constant J6DATADD2: STD_LOGIC_VECTOR:="010101"; -- write to J6DAT. 3D5
constant J6DATADD3: STD_LOGIC_VECTOR:="010110"; -- write to J6DAT. 3D6
constant J6DATADD4: STD_LOGIC_VECTOR:="010111"; -- write to J6DAT. 3D7
begin

CS <= (NOT NSS1) and (NOT NAA3); -- chip select.
read <= NOT NRD;
write <= NOT NWR;
D <= seq when (read = '1') and (CS = '1') and (A = SEQREGADD) else -- allow readback of seq register
	"ZZZZZZZZZZZZZZZZZZZZ";
BA <= A;
NBRD <= NRD;
NBWR <= NWR;
BRW <= '1' when (write='1') and (NAA3 = '0') else '0';

-- Sequence register!!
-- we do something a little different than a simple register.
-- one main address allows straight write access...
-- 3 aux addresses allow special access to the sequence register
-- they allow the setting and clearing of individual bits
-- and also a 12 bit address register that any value can be added to.
-- 

process ( NWR, NRESET, CS )

  begin
  
  SEQ <= SEQ;  
  if NRESET='0' then
	SEQ <= "00000000000000000000";
  elsif NWR'event and NWR ='1' and CS = '1' then -- on rising edge of write strobe. 
	case A is 
		when SEQREGADD =>
			SEQ <= D ;
		when SEQORADD =>
			SEQ <= SEQ or D; -- set bits written as a '1', others unchanged.
		when SEQNANDADD =>
			SEQ <= SEQ and NOT D; -- clear bits written as a '1', others unchanged 
		when SEQTOGADD => 
			SEQ <= SEQ xor D; -- toggle bits written as a '1', others unchanged
		when SEQSTATADD =>
			SEQ(2) <= D(2); -- spare TTL
			SEQ(3) <= D(3); -- spare TTL
		when SEQFRMADD =>
			SEQ(0) <= D(0); -- ADC convert clock
			SEQ(1) <= D(1); -- FRAME bit
			SEQ(4) <= D(4); -- IPCA and IPCB (vdduc / vssuc)
 			SEQ(5) <= D(5); -- RGA and RGB (vpd / vnd)
 			SEQ(12) <= D(12); -- SWA SWB (Iidle / Islew)
 			SEQ(13) <= D(13); -- TGA TGB (Vdetcom / Vddout)
		when SEQCOLADD =>
			SEQ(6) <= D(6); -- Fast Phase 1
			SEQ(7) <= D(7); -- Fast Phase 2
			SEQ(8) <= D(8); -- Fast Sync
			SEQ(18) <= D(18); -- Fast Sync
			SEQ(19) <= D(19); -- Fast Sync
		when SEQROWADD =>
			SEQ(9) <= D(9); -- Slow Phase 1
			SEQ(10) <= D(10); -- Slow Phase 2
			SEQ(11) <= D(11); -- Slow Sync
			SEQ(14) <= D(14); -- VggCL
			SEQ(15) <= D(15); -- spare, could be prSExEn
			SEQ(16) <= D(16); -- pReset
			SEQ(17) <= D(17); -- pGlobal
		when others =>
			
	end case;
end if;

end process;

process (clk, write)
begin
  writedlyd <= writedlyd;
  if write = '0' then
	writedlyd <= '0';
  elsif clk'event and clk ='1' then -- rising edge of clock.
	writedlyd <= write;	
  end if;
end process;

shiftout <= j6reg(0);

process (clk)

begin
  j6reg <= j6reg;
  if clk'event and clk ='1' then -- rising edge of clock.
    if (write = '1') and (A(5 downto 2) = "0101") and (CS = '1') and shiften = '0' then
		case A(1 downto 0) is
			when "00" =>
				j6reg(17 downto 0) <= '1' & D(15 downto 0) & '0'; --  stop, data, start
			when "01" =>
				j6reg(17 downto 0) <= '1' & not D(15) & D(14 downto 0) & '0'; --  stop, data, start
			when "10" =>
			    j6reg(35 downto 18) <= '1' & D(15 downto 0) & '0'; -- stop, data, start
			when "11" =>
			    j6reg(35 downto 18) <= '1' & not D(15) & D(14 downto 0) & '0'; -- stop, data, start
		end case;
	elsif shiften = '1' then -- fill with stop bits as data shifts out.
		j6reg <= '1' & j6reg(35 downto 1);
	end if;
  end if;
end process;

end U9_arch;

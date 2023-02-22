----------------------------------------------------------------------------------
-- Company: University of Strathclyde
-- Engineer: Logan Noonan
-- 
-- Create Date: 27.01.2023 10:07:44
-- Design Name: 
-- Module Name: comb_logic - Behavioral
-- Target Devices: BASYS 3
-- Description: PRACTICAL LAB #3

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity comb_logic is 
    PORT(
        V : in STD_LOGIC;
        W : in STD_LOGIC;
        X : in STD_LOGIC;
        Y : in STD_LOGIC;
        Z : in STD_LOGIC;
        OP : out STD_LOGIC
        );
end entity comb_logic;

architecture ARCH1 of comb_logic is

    component AOI
        port(A,B,C,D : in STD_LOGIC;
             F : out STD_LOGIC);
    end component;
    
    signal intA, intB : STD_LOGIC;

begin     

AOI_A : AOI
port map(A => V, B  => W, C => X, D => Y, F => intA);

AOI_B : AOI
port map(A => Y, B  => X, C => V, D => Z, F => intB);

OP <= intA and intB;
    
end ARCH1;


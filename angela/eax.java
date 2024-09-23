/*
**	rs.adria.sec.Eax
**
**	Copyright (c) 2008-2012, RedStar Technologies, All rights reserved.
**	http://www.redstar-technologies.com/adria/
**
**	THIS LIBRARY IS PROVIDED BY REDSTAR TECHNOLOGIES "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
**	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
**	PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL REDSTAR TECHNOLOGIES BE LIABLE FOR ANY
**	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
**	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
**	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
**	STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
**	USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

package rs.adria.sec;

/**
**	Enhanced Angela encryption algorithm implementation.
*/

public class Eax
{
	/**
	**	Number of bytes for the block buffers and the counter mask.    
	*/
	public static final int BLOCK_SIZE = 32;
	private static final int MASK = 31;

	/**
	**	Initial constants for the P and Q vectors. 
	*/
	private static int[] consts = {
		67, 53, 419, 709, 373, 101, 401, 761, 997, 739, 641, 313, 61, 83, 59, 769,
		1129, 2969, 4937, 5743, 7237, 6571, 8167, 8713, 8933, 5179, 3673, 3727, 3083, 4817, 4523, 4507
	};

	/**
	**	Context input and output buffers. 
	*/
	private byte[] in, out; 

	/**
	**	P and Q parameter vectors. 
	*/
	private int[] p, q;

	/**
	**	Current keyword used in the algorithm. 
	*/
	private byte[] kw;

	/**
	**	Input buffer level, keyword length and current byte index. 
	*/
	private int level, kl, ki;

	/**
	**	Default internal context for encryption and decryption.
	*/
	private static Eax defContext = new Eax (null);

	/**
	**	Rotates an integer value V of M-bits to the left by N bits.  
	*/
	private static int rol (int v, int n, int m)
	{
		v &= (1 << m) - 1;
		return ((v << n) | ((v >> (m-n)) & ((1 << n) - 1))) & ((1 << m) - 1);
	}

	/**
	**	Rotates an integer value V of M-bits to the right by N bits.  
	*/
	private static int ror (int v, int n, int m)
	{
		return rol (v, m-n, m);
	}

	/**
	**	Constructs the context and performs initial updates on the internal parameters for
	**	the specified keyword.  
	*/
	public Eax (byte[] keyword)
	{
		in = new byte[BLOCK_SIZE];
		out = new byte[BLOCK_SIZE];

		p = new int[BLOCK_SIZE];
		q = new int[BLOCK_SIZE];

		if (keyword != null) reset (keyword);
	}

	/**
	**	Resets the state and performs initial updates on the internal parameters to prepare
	**	them with the specified keyword.  
	*/
	public void reset (byte[] keyword)
	{
		int i, j, sumL, sumH;

		level = ki = 0;

		kw = keyword;
		kl = kw.length;

		for (i = 0; i < BLOCK_SIZE; i++)
		{
			p[i] = consts[i];
			q[i] = consts[BLOCK_SIZE-i-1];
		}

		if (kl != 0)
		{
            for (i = j = 0; i < kl; i++)
            	j += 3*kw[i];
		}
		else
			j = 0xA527;

		sumH = (j >> 8) & 0xFF;
		sumL = j & 0xFF;

		for (i = 0; i < BLOCK_SIZE; i++)
		{
			p[i&MASK] = ror (p[i] + sumH, sumH & 5, 16) - p[(i+1) & MASK];
			q[i&MASK] = rol (q[i] + q[(i+1) & MASK], sumL & 3, 16) - sumL;

			j = p[i] & q[i];

			sumH += (j & 0xF0);
			sumL -= (j & 0x0F);
		}
	}

	/**
	**	Returns true if the context's input buffer is full.
	*/
	public boolean isInputFull ()
	{
		return level == BLOCK_SIZE;
	}

	/**
	**	Returns true if the context's input buffer is empty.
	*/
	public boolean isInputEmpty ()
	{
		return level == 0;
	}

	/**
	**	Returns number of free spaces left in the input.
	*/
	public int inputSpace ()
	{
		return BLOCK_SIZE - level;
	}

	/**
	**	Returns the output block buffer.
	*/
	public byte[] getOutput ()
	{
		return out;
	}

	/**
	**	Returns the input block buffer.
	*/
	public byte[] getInput ()
	{
		return in;
	}

	/**
	**	Returns the output aligned size of an input buffer of N bytes. 
	*/
	public static int getOutputSize (int n)
	{
		return (n + BLOCK_SIZE - 1) & -BLOCK_SIZE;
	}

	/**
	**	Returns the valid number of actual data bytes in the last block for an input of N bytes. 
	*/
	public static int getBottomBytes (int n)
	{
		return (n & MASK) != 0 ? (n & MASK) : BLOCK_SIZE;
	}

	/**
	**	Encrypts the block in the input buffer and results are written into the output buffer. The
	**	output buffer is returned.  
	*/
	public byte[] encryptBlock ()
	{
		int i, k, k2;

		if (kl != 0)
		{
			for (i = 0; i < BLOCK_SIZE; i++)
			{
				q[i] = rol (q[i] + kw[ki], kw[ki] & 3, 16);
				ki = ++ki % kl;
			}
		}

		for (i = 0; i < BLOCK_SIZE; i++)
			q[i] = ror (q[i], q[i] & 5, 16) ^ rol (p[i], p[i] & 7, 16);

		for (i = 0; i < BLOCK_SIZE; i++)
		{
			k = in[i];

			k2 = rol(k, 4, 8) ^ p[i];
			k2 = k2 + ror(q[BLOCK_SIZE-i-1], p[i] & 3, 8);

			k &= 0xFF; k2 &= 0xFF;
			p[i] ^= (k + k2);
			out[i] = (byte)k2;
		}

		level = 0;
		return out;
	}

	/**
	**	Decrypts the block in the input buffer the results are written to the output buffer. The
	**	output buffer is returned.  
	*/
	public byte[] decryptBlock ()
	{
		int i, k, k2;

		if (kl != 0)
		{
			for (i = 0; i < BLOCK_SIZE; i++)
			{
				q[i] = rol (q[i] + kw[ki], kw[ki] & 3, 16);
				ki = ++ki % kl;
			}
		}

		for (i = 0; i < BLOCK_SIZE; i++)
			q[i] = ror (q[i], q[i] & 5, 16) ^ rol (p[i], p[i] & 7, 16);

		for (i = 0; i < BLOCK_SIZE; i++)
		{
			k2 = in[i];

			k = k2 - ror(q[BLOCK_SIZE-i-1], p[i] & 3, 8);
			k = ror(k ^ p[i], 4, 8);

			k &= 0xFF; k2 &= 0xFF;
			p[i] ^= (k + k2);
			out[i] = (byte)k;
		}

		level = 0;
		return out;
	}

	/**
	**	Feeds a byte value into the input buffer of the context. Returns false if the input
	**	buffer is full and the value was not added.  
	*/
	public boolean feed (int value)
	{
		if (level == BLOCK_SIZE) return false;

		in[level++] = (byte)((value + 0x100) & 0xFF);
		return true;
	}

	/**
	**	Shortcut function to encrypt byte buffers. Returns the EAX encrypted result.  
	*/
	public static byte[] encrypt (byte[] keyword, byte[] value)
	{
		try
		{
			defContext.reset(keyword);

			byte[] b_dest = new byte[Eax.getOutputSize(value.length)];
			int d = 0;

			for (int i = 0; i < value.length; i++)
			{
				if (!defContext.feed(value[i]))
				{
					defContext.encryptBlock();

					System.arraycopy(defContext.getOutput(), 0, b_dest, d, BLOCK_SIZE);
					d += BLOCK_SIZE;

					defContext.feed(value[i]);
				}
			}

			if (!defContext.isInputEmpty())
			{
				defContext.encryptBlock();
				System.arraycopy(defContext.getOutput(), 0, b_dest, d, BLOCK_SIZE);
			}

			return b_dest;
		}
		catch (Exception e) {
		}

		return null;
	}

	/**
	**	Shortcut function to decrypt byte buffers. Returns the EAX decrypted result.  
	*/
	public static byte[] decrypt (byte[] keyword, byte[] value)
	{
		try
		{
			defContext.reset(keyword);

			byte[] b_dest = new byte[Eax.getOutputSize(value.length)];
			int d = 0;

			for (int i = 0; i < value.length; i++)
			{
				if (!defContext.feed(value[i]))
				{
					defContext.decryptBlock();

					System.arraycopy(defContext.getOutput(), 0, b_dest, d, BLOCK_SIZE);
					d += BLOCK_SIZE;

					defContext.feed(value[i]);
				}
			}

			if (!defContext.isInputEmpty())
			{
				defContext.decryptBlock();
				System.arraycopy(defContext.getOutput(), 0, b_dest, d, BLOCK_SIZE);
			}

			return b_dest;
		}
		catch (Exception e) {
		}

		return null;
	}
};

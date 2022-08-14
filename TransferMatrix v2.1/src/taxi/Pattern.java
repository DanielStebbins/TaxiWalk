package taxi;

public class Pattern implements Comparable<Pattern>
{
	// Read right to left.
	public long steps;
	public byte length;
	
	public Pattern(long steps, byte length)
	{
		this.steps = steps;
		this.length = length;
	}

	@Override
	public int compareTo(Pattern o)
	{
//		System.out.println("Compare Lengths: " + length + ", " + o.length);
		if(length != o.length)
		{
			return length - o.length;
		}
//		System.out.println("Compare Steps: " + Long.toBinaryString(steps) + ", " + Long.toBinaryString(o.steps));
		// Stolen from Java Long.
		return (steps < o.steps) ? -1 : ((steps == o.steps) ? 0 : 1);
	}
	
	@Override
	public String toString()
	{
		return "Length: " + length + ", " + Long.toBinaryString(steps);
	}
}

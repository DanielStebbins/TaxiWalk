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
		if(length != o.length)
		{
			return length - o.length;
		}
		// Stolen from Java Long.
		return (steps < o.steps) ? -1 : ((steps == o.steps) ? 0 : 1);
	}
	
	@Override
	public String toString()
	{
		return "Length: " + length + ", " + Long.toBinaryString(steps);
	}
}

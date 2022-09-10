package taxi;

public class State implements Comparable<State>
{
	public Pattern pattern;
	public State horizontal;
	public State vertical;
	
	public State(Pattern pattern)
	{
		this.pattern = pattern;
	}

	@Override
	public int compareTo(State o)
	{
		return pattern.compareTo(o.pattern);
	}
	
	@Override
	public String toString()
	{
		return "\"" + pattern + "\"";
	}
}

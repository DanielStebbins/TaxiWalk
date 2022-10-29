// Record: N=47 in 7.8 seconds.
// Record: N=51 in 58 seconds.

package taxi;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedList;

public class TaxiWalk
{
	// The length of walk to enumerate. MAX 64 CURRENTLY (long encoding).

	public static final int N = 43;
	
	// These constants relate to the previously calculated steps to the origin file.
	public static final int MAX_N = 100;
	public static final int DIM = 2 * MAX_N + 1;
	public static final int OFFSET = DIM * DIM;
	public static final int LATTICE_SIZE = OFFSET * 4;
	public static int[] stepsToOrigin = new int[LATTICE_SIZE];
	
	// Used for building the automaton.
	public static State genesis = new State(0L, (byte) 0);
	public static State twoNullPointers = new State(0L, (byte) 0);
	public static int size = 1;
	public static LinkedList<State> untreated = new LinkedList<State>();
	
	public static long findEndpoint = 0;
	public static long hasLoop = 0;
	public static long reduce = 0;
	public static long contains = 0;
	public static long runAutomaton = 0;
	
	public static void main(String args[])
	{
		// Read the steps to the origin from every point, which have been previously calculated and stored.
		try
		{
			BufferedReader reader = new BufferedReader(new FileReader(new File("StepsToOrigin.txt")));
			String[] split = reader.readLine().split(" ");
			for(int i = 0; i < LATTICE_SIZE; i++)
			{
				stepsToOrigin[i] = Integer.parseInt(split[i]);
			}
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		
		for(int n = 39; n <= N; n += 4)
		{
			long startTime = System.currentTimeMillis();
			
			// Set up the automaton stuff.
			untreated.addLast(genesis);
			int count = 0;
			twoNullPointers.index = count;
			count++;
			
			// Main automaton-generating code.
			long steps = 0L;
			byte length = 0;
			while(!untreated.isEmpty())
			{
				State start = untreated.removeFirst();
				
				// Try to take a horizontal step.
				if(start.length < 2 || approach(start.steps, start.length) != 1)
				{
					steps = start.steps;
					length = (byte) (start.length + 1);
					
					// Find end point.
					long time = System.currentTimeMillis();
					int endX = 0;
					int xStep = 1;
					int endY = 0;
					int yStep = 1;
					for(int i = 0; i < length; i++)
					{
						if((steps >>> i & 1) == 0)
						{	
							endX += xStep;
							yStep = -yStep;
						}
						else
						{
							endY += yStep;
							xStep = -xStep;
						}
					}
					findEndpoint += System.currentTimeMillis() - time;
					
					
					// Check for loop.		
					if(!hasLoop(steps, length, endX, endY))
					{
						// If cannot loop, chop off first step.
						time = System.currentTimeMillis();
						while(stepsToOrigin[approach(steps, length) * OFFSET + (endX + MAX_N) * DIM + endY + MAX_N] > n - length)
						{
							if((steps & 1) == 0)
							{
								endX -= 1;
								endY = -endY;
							}
							else
							{
								endX = -endX;
								endY -= 1;
							}
							steps = steps >>> 1;
							length--;
							
							if((steps & 1) == 1)
							{
								steps = steps ^ ((1L << length) - 1);
								int temp = endX;
								endX = endY;
								endY = temp;
							}
						}
						
						// If first step is now vertical, flip to horizontal.
						if((steps & 1) == 1)
						{
							steps = steps ^ ((1L << length) - 1);
						}
						reduce += System.currentTimeMillis() - time;
						
						
						time = System.currentTimeMillis();
						State temp = getState(steps, length);
						if(temp == null)
						{
							// Added to tree.
							State end = new State(steps, length);
							start.horizontal = end;
							untreated.addLast(end);
							
						}
						else
						{
							// Present in tree.
							start.horizontal = temp;
						}
						contains += System.currentTimeMillis() - time;
					}
				}
				
				
				// Try to take a vertical step.
				if(start.length < 2 || approach(start.steps, start.length) != 2)
				{
					steps = start.steps | (1L << start.length);
					length = (byte) (start.length + 1);
				
					// Find end point.
					long time = System.currentTimeMillis();
					int endX = 0;
					int xStep = 1;
					int endY = 0;
					int yStep = 1;
					for(int i = 0; i < length; i++)
					{
						if((steps >>> i & 1) == 0)
						{	
							endX += xStep;
							yStep = -yStep;
						}
						else
						{
							endY += yStep;
							xStep = -xStep;
						}
					}
					findEndpoint += System.currentTimeMillis() - time;
					
					
					// Check for loop.		
					if(!hasLoop(steps, length, endX, endY))
					{
						// If cannot loop, chop off first step.
						time = System.currentTimeMillis();
						while(stepsToOrigin[approach(steps, length) * OFFSET + (endX + MAX_N) * DIM + endY + MAX_N] > n - length)
						{
							if((steps & 1) == 0)
							{
								endX -= 1;
								endY = -endY;
							}
							else
							{
								endX = -endX;
								endY -= 1;
							}
							steps = steps >>> 1;
							length--;
							
							if((steps & 1) == 1)
							{
								steps = steps ^ ((1L << length) - 1);
								int temp = endX;
								endX = endY;
								endY = temp;
							}
						}
						
						// If first step is now vertical, flip to horizontal.
						if((steps & 1) == 1)
						{
							steps = steps ^ ((1L << length) - 1);
						}
						reduce += System.currentTimeMillis() - time;
						
						
						time = System.currentTimeMillis();
						State temp = getState(steps, length);
						if(temp == null)
						{
							// Added to tree.
							State end = new State(steps, length);
							start.vertical = end;
							untreated.addLast(end);
						}
						else
						{
							// Present in tree.		
							start.vertical = temp;
						}
						contains += System.currentTimeMillis() - time;
					}
				}

				if(start.vertical != null && start.vertical.steps == 0b1001110001100111000L && start.vertical.length == 19)
				{
					System.out.println("\n" + start);
					System.out.println(start.horizontal);
					System.out.println(start.vertical);
				}
//				
//				if(start.steps == 0b001110001111100L && start.length == 15)
//				{
//					System.out.println("\n" + start);
//					System.out.println(start.horizontal);
//					System.out.println(start.vertical);
//				}
		
				// Reducing Tree
				if(start.horizontal == null && start.vertical == null)
				{
//					System.out.println("\nDeleting: " + start);
//					System.out.println("S Parent: " + start.parent);
//					System.out.println("S Horizontal: " + start.parent.horizontal);
//					System.out.println("S Vertical: " + start.parent.vertical);
					if(start == start.parent.horizontal)
					{
						start.parent.horizontal = twoNullPointers;
					}
					else
					{
						start.parent.vertical = twoNullPointers;
					}
					if(start.parentCount > 1)
					{
						System.out.println(start + ": " + start.parentCount);
					}
//					System.out.println("E Parent: " + start.parent);
//					System.out.println("E Horizontal: " + start.parent.horizontal);
//					System.out.println("E Vertical: " + start.parent.vertical);
				}
				else if(start.horizontal == null)
				{
					if(start.vertical.oneNullParent == null)
					{
						start.index = count;
						count++;
						start.vertical.oneNullParent = start;
						if(start.vertical.parent == null)
						{
							start.vertical.parent = start;
						}
//						start.vertical.parentCount += 1;
//						System.out.println("2-1V: " + start.vertical.parentCount);
					}
					else
					{
//						if(start == start.parent.horizontal)
//						{
//							start.parent.horizontal = start.vertical.oneNullParent;
//						}
//						else
//						{
//							start.parent.vertical = start.vertical.oneNullParent;
//						}
						
//						start.vertical.oneNullParent.parentCount += 1;
//						System.out.println("2-2V: " + start.vertical.oneNullParent.parentCount);
						if(start.parentCount > 1)
						{
							System.out.println(start + ": " + start.parentCount);
						}
					}
				}
				else if(start.vertical == null)
				{
					if(start.horizontal.oneNullParent == null)
					{
						start.index = count;
						count++;
						start.horizontal.oneNullParent = start;
						if(start.horizontal.parent == null)
						{
							start.horizontal.parent = start;
						}
//						start.horizontal.parentCount += 1;
//						System.out.println("3-1H: " + start.horizontal.parentCount);
					}
					else
					{
//						if(start.steps == 0b111100011111000L && start.length == 15)
//						{
//							System.out.println(start.horizontal.parent);
//							System.out.println(start.horizontal.parent.horizontal);
//							System.out.println(start.horizontal.parent.vertical);
//						}
						
//						if(start == start.parent.horizontal)
//						{
//							start.parent.horizontal = start.horizontal.oneNullParent;
//						}
//						else
//						{
//							start.parent.vertical = start.horizontal.oneNullParent;
//						}
						
//						start.horizontal.oneNullParent.parentCount += 1;
//						System.out.println("3-2H: " + start.horizontal.oneNullParent.parentCount);
						
						if(start.parentCount > 1)
						{
							System.out.println(start + ": " + start.parentCount);
						}
					}
				}
				else
				{
					start.index = count;
					count++;
					start.horizontal.parentCount += 1;
					if(start.vertical != start.horizontal)
					{
						start.vertical.parentCount += 1;
					}
//					System.out.println("4H: " + start.horizontal.parentCount);
//					System.out.println("4V: " + start.vertical.parentCount);
					if(start.horizontal.parent == null)
					{
						start.horizontal.parent = start;
					}
					if(start.vertical.parent == null)
					{
						start.vertical.parent = start;
					}
				}
				
//				if(start.steps == 0b1001110001100111000L && start.length == 19)
//				{
//					System.out.println("\n" + start.parent);
//					System.out.println(start.parent.horizontal);
//					System.out.println(start.parent.vertical);
//				}
			}
			
			// Running the automaton.
			long time = System.currentTimeMillis();
			LinkedList<State> current = new LinkedList<State>();
			LinkedList<State> next = new LinkedList<State>();
			int[] currentCounts = new int[count];
			int[] nextCounts = new int[count];
			
			currentCounts[genesis.index] = 1;
			current.addLast(genesis);
			
			for(int i = 1; i <= n; i++)
			{
//				System.out.println("\n" + i);
				while(!current.isEmpty())
				{
					State start = current.removeFirst();
					
					if(start.steps == 0b0011100011001110000L && start.length == 19)
					{
						System.out.println("\n" + start);
						System.out.println(start.horizontal);
						System.out.println(start.vertical);
						System.out.println(start.vertical.horizontal);
						System.out.println(start.vertical.vertical);
					}
//					
//					if(start.steps == 0b001110001111100L && start.length == 15)
//					{
//						System.out.println("\n" + start);
//						System.out.println(start.horizontal);
//						System.out.println(start.vertical);
//					}
					
//					System.out.println("\n" + start);
					
					if(start.horizontal != null)
					{
//						System.out.println(start.horizontal);
						if(nextCounts[start.horizontal.index] == 0)
						{
							next.addLast(start.horizontal);
						}
						nextCounts[start.horizontal.index] += currentCounts[start.index];
					}
					
					if(start.vertical != null)
					{
//						System.out.println(start.vertical);
						if(nextCounts[start.vertical.index] == 0)
						{
							next.addLast(start.vertical);
						}
						nextCounts[start.vertical.index] += currentCounts[start.index];
					}
				}
				current = next;
				next = new LinkedList<State>();
				
				currentCounts = nextCounts;
				nextCounts = new int[count];
			}
	
			long taxi = 0;
			while(!current.isEmpty())
			{
				taxi += currentCounts[current.removeFirst().index];
			}
			runAutomaton = System.currentTimeMillis() - time;
			
			
			// Output Statistics.
			long endTime = System.currentTimeMillis();
			System.out.println("\nN: " + n);
			System.out.println("Automaton Size: " + count);
			System.out.println("Number of Taxi Walks: " + taxi);
			System.out.println("Total Time: " + (endTime - startTime) / 1000.0 + "\n");
			
			System.out.println("Find Endpoint: " + findEndpoint / 1000.0 + "(" + Math.round((double) findEndpoint / (endTime - startTime) * 1000) / 10.0 + "%)");
			System.out.println("Has Loop: " + hasLoop / 1000.0 + "(" + Math.round((double) hasLoop / (endTime - startTime) * 1000) / 10.0 + "%)");
			System.out.println("Reduce Pattern: " + reduce / 1000.0 + "(" + Math.round((double) reduce / (endTime - startTime) * 1000) / 10.0 + "%)");
			System.out.println("Tree Contains: " + contains / 1000.0 + "(" + Math.round((double) contains / (endTime - startTime) * 1000) / 10.0 + "%)");
			System.out.println("Running the Automaton: " + runAutomaton / 1000.0 + "(" + Math.round((double) runAutomaton / (endTime - startTime) * 1000) / 10.0 + "%)");
			
			
			genesis = new State(0L, (byte) 0);
			count = 0;
			untreated.clear();
			
			findEndpoint = 0;
			hasLoop = 0;
			reduce = 0;
			contains = 0;
			runAutomaton = 0;
		}
	}
	
	public static int approach(long steps, byte length)
	{
		return (int) ((steps >>> (length - 2) & 1) * 2 + (steps >>> (length - 1)));
	}
	
	public static boolean hasLoop(long steps, byte length, int endX, int endY)
	{
		long time = System.currentTimeMillis();
		int x = 0;
		int xStep = 1;
		int y = 0;
		int yStep = 1;
		boolean loop = x == endX && y == endY;
		int i = 0;
		while(!loop && i < length - 12)
		{					
			if((steps >>> i & 1) == 0)
			{	
				x += xStep;
				yStep = -yStep;
			}
			else
			{
				y += yStep;
				xStep = -xStep;
			}
			loop = x == endX && y == endY;
			i++;
		}
		hasLoop += System.currentTimeMillis() - time;
		return loop;
	}
	
	public static State getState(long steps, byte length)
	{
		State parent = genesis;
		for(int i = 0; i < length - 1; i++)
		{
			if((steps & 1) == 1)
			{
				parent = parent.vertical;
			}
			else
			{
				parent = parent.horizontal;
			}
			steps >>= 1;
		}
		
		if(steps == 1)
		{
			return parent.vertical;
		}
		else
		{
			return parent.horizontal;
		}
	}
}

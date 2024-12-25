namespace lab7;

internal static class Program
{
    private static async Task Main()
    {
        Console.Write("Enter the path to the text file: ");
        var filePath = Console.ReadLine();

        Console.Write("Enter characters to remove (without spaces): ");
        var charsToRemove = Console.ReadLine();

        try
        {
            var content = await File.ReadAllTextAsync(filePath ?? "");

            var result = new string(content.Where(c => !charsToRemove?.Contains(c) ?? true).ToArray());

            await File.WriteAllTextAsync(filePath ?? "", result);

            Console.WriteLine("Characters successfully removed and file saved.");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"An error occurred: {ex.Message}");
        }
    }
}
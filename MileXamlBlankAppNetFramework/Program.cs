using MileXamlBlankAppNetFramework;
using System;
using System.Windows.Forms;

namespace MileXamlBlankAppNetFrameworkModern
{
    public static class Program
    {
        [STAThread]
        static void Main()
        {
            //Application.EnableVisualStyles();
            //Application.SetCompatibleTextRenderingDefault(false);
            App app = new App();
            app.InitializeComponent();
            //app.Run();

           
            //Application.Run(new Form1());

            //app.Close();
        }
    }
}

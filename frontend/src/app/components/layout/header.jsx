import { Search } from "lucide-react";

export default function Header() {
  return (
    <header className="fixed top-0 left-0 w-full h-16 bg-gradient-to-r from-blue-900 to-blue-800 border-b border-blue-950 shadow-lg z-50 flex items-center justify-between px-4 sm:px-6">
      {/* Left Section: Logo & Title */}
      <div className="flex items-center gap-3 shrink-0 mr-4">
        <div className="bg-white/10 p-1.5 rounded-lg border border-white/20 backdrop-blur-sm flex items-center justify-center shadow-inner">
          <img src="/favicon.ico" alt="Logo" className="w-7 h-7 object-contain" />
        </div>
        <h1 className="text-white font-extrabold text-lg sm:text-xl tracking-wide whitespace-nowrap drop-shadow-md">
          MOP Simulator
        </h1>
      </div>

      {/* Right Section: Search Bar */}
      <div className="flex flex-1 justify-end max-w-md">
        <div className="relative w-full transition-all duration-300 group">
          <div className="absolute inset-y-0 left-0 pl-3 flex items-center pointer-events-none">
            <Search className="h-4 w-4 text-blue-300 group-focus-within:text-blue-500 transition-colors" />
          </div>
          <input
            className="w-full py-2 pl-10 pr-4 bg-blue-950/40 border border-blue-700/50 rounded-full text-sm text-white placeholder-blue-300 focus:outline-none focus:bg-white focus:text-slate-900 focus:border-white focus:ring-4 focus:ring-blue-500/30 transition-all duration-300 shadow-inner"
            type="text"
            placeholder="Search..."
          />
        </div>
      </div>
    </header>
  );
}

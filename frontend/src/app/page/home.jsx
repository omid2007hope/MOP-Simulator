export default function HomePage() {
  return (
    <div className="flex-1 w-full pt-20 px-4 sm:px-6 lg:px-8 pb-8 bg-slate-50 overflow-y-auto">
      <div className="max-w-7xl mx-auto grid grid-cols-1 lg:grid-cols-2 gap-6 lg:gap-8 h-full min-h-[600px]">
        
        {/* Input Section */}
        <div className="bg-white rounded-2xl shadow-sm border border-slate-200 overflow-hidden flex flex-col h-full">
          <div className="px-6 py-4 border-b border-slate-100 bg-slate-50/50">
            <h2 className="text-lg font-semibold text-slate-800">Configuration</h2>
          </div>
          
          <div className="p-6 flex-1 flex flex-col gap-6">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-5">
              {/* Title */}
              <div className="flex flex-col gap-1.5">
                <label className="text-sm font-semibold text-slate-700">Title</label>
                <input 
                  type="text" 
                  placeholder="Enter title..."
                  className="px-3.5 py-2.5 border border-slate-300 rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-500/20 focus:border-blue-500 transition-all text-sm bg-slate-50/50 focus:bg-white"
                />
              </div>
              
              {/* Full Name */}
              <div className="flex flex-col gap-1.5">
                <label className="text-sm font-semibold text-slate-700">Full Name</label>
                <input 
                  type="text" 
                  placeholder="Enter full name..."
                  className="px-3.5 py-2.5 border border-slate-300 rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-500/20 focus:border-blue-500 transition-all text-sm bg-slate-50/50 focus:bg-white"
                />
              </div>
              
              {/* Cycle */}
              <div className="flex flex-col gap-1.5 md:col-span-2">
                <label className="text-sm font-semibold text-slate-700">Cycle</label>
                <input 
                  type="text" 
                  placeholder="Enter cycle..."
                  className="px-3.5 py-2.5 border border-slate-300 rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-500/20 focus:border-blue-500 transition-all text-sm bg-slate-50/50 focus:bg-white"
                />
              </div>
            </div>
            
            {/* Description */}
            <div className="flex flex-col gap-1.5 flex-1">
              <label className="text-sm font-semibold text-slate-700">Description</label>
              <textarea 
                placeholder="Enter description details..."
                className="w-full flex-1 min-h-[140px] px-3.5 py-2.5 border border-slate-300 rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-500/20 focus:border-blue-500 transition-all text-sm bg-slate-50/50 focus:bg-white resize-none"
              />
            </div>
            
            <button className="mt-2 w-full py-3 bg-blue-600 hover:bg-blue-700 text-white font-semibold rounded-xl shadow-sm hover:shadow transition-all duration-200 focus:outline-none focus:ring-4 focus:ring-blue-500/30 active:scale-[0.98]">
              Run Simulation
            </button>
          </div>
        </div>

        {/* Output Section */}
        <div className="bg-white rounded-2xl shadow-sm border border-slate-200 overflow-hidden flex flex-col h-full min-h-[400px]">
          <div className="px-6 py-4 border-b border-slate-100 bg-slate-50/50 flex justify-between items-center">
            <h2 className="text-lg font-semibold text-slate-800">Output</h2>
            <span className="text-xs font-semibold px-2.5 py-1 bg-slate-200 text-slate-600 rounded-full">
              Waiting for input
            </span>
          </div>
          
          <div className="p-6 flex-1 bg-slate-50/30 flex items-center justify-center">
            {/* Empty State Placeholder */}
            <div className="text-center flex flex-col items-center justify-center text-slate-400 p-8 border-2 border-dashed border-slate-200 rounded-2xl w-full h-full">
              <svg className="w-12 h-12 mb-4 text-slate-300" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={1.5} d="M19.428 15.428a2 2 0 00-1.022-.547l-2.387-.477a6 6 0 00-3.86.517l-.318.158a6 6 0 01-3.86.517L6.05 15.21a2 2 0 00-1.806.547M8 4h8l-1 1v5.172a2 2 0 00.586 1.414l5 5c1.26 1.26.367 3.414-1.415 3.414H4.828c-1.782 0-2.674-2.154-1.414-3.414l5-5A2 2 0 009 10.172V5L8 4z" />
              </svg>
              <h3 className="text-sm font-semibold text-slate-500 mb-1">No Results Yet</h3>
              <p className="text-sm text-slate-400 max-w-xs">
                Configure the parameters on the left and run the simulation to see the output here.
              </p>
            </div>
          </div>
        </div>

      </div>
    </div>
  );
}

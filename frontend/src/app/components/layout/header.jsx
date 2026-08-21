import { Search } from "lucide-react";

export default function Header() {
  return (
    <header className="flex flex-row w-full h-9/100 bg-blue-900 z-250 fixed">
      {/* logo */}
      <div className="h-full w-4/100 flex flex-col justify-around items-center text-white font-bold">
        <img src="../../favicon.ico" alt="" />
      </div>
      {/* title */}
      <div className="h-full w-36/100 flex flex-col justify-around items-center text-white font-bold">
        <h1 className="">MOP Simulator</h1>
      </div>
      {/* search bar  */}
      <div className="h-full w-60/100 flex flex-row justify-end  items-center text-white font-bold">
        <button className="p-1.5 border-y border-l rounded-l-md">
          <Search size={20} />
        </button>
        <input
          className="py-1 pl-2.5 pr-35 border-y border-r rounded-r-md mr-2"
          type="text"
        />
      </div>
    </header>
  );
}

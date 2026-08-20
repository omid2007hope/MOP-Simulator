export default function Header() {
  return (
    <header className="flex flex-row w-full h-9/100 bg-blue-900 border-b-2 border-black z-250 fixed">
      {/* logo */}
      <div className="h-full w-4/100 flex flex-col justify-around items-center text-white font-bold border">
        <img src="" alt="" />
      </div>
      {/* title */}
      <div className="h-full w-36/100 flex flex-col justify-around items-center text-white font-bold border">
        <h1 className=""></h1>
      </div>
      {/* search bar  */}
      <div className="h-full w-60/100 flex flex-col justify-around items-center text-white font-bold border">
        <button className=""></button>
        <input className=" " type="text" />
      </div>
    </header>
  );
}

async function loadTasks() {
  let res = await fetch('/api/tasks');
  let tasks = await res.json();
  ['development','therapy','preservation'].forEach(id=>{
    document.getElementById(id).innerHTML='';
  });
  tasks.forEach((t, i)=>{
    let li=document.createElement('li');
    li.textContent=t.title;
    if(t.done) li.classList.add('done');
    li.onclick=()=>toggleTask(i);
    if(t.category==='Organic Development') document.getElementById('development').appendChild(li);
    if(t.category==='Organic Therapy') document.getElementById('therapy').appendChild(li);
    if(t.category==='Organic Preservation') document.getElementById('preservation').appendChild(li);
  });
}

async function addTask() {
  let title=document.getElementById('taskInput').value;
  let cat=document.getElementById('categorySelect').value;
  await fetch('/api/tasks',{method:'POST',body:JSON.stringify({title,category:cat}),headers:{'Content-Type':'application/json'}});
  document.getElementById('taskInput').value='';
  loadTasks();
}

async function toggleTask(i) {
  await fetch('/api/toggle',{method:'POST',body:JSON.stringify({index:i}),headers:{'Content-Type':'application/json'}});
  loadTasks();
}

loadTasks();

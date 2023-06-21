<template>
    <div class="home">
        <div class="title">Last Update: 
           <p id="intext">{{this.last_update}}</p > 
        </div>
        <div class="title">Temperature:  
            <p id="intext">{{this.temperature}}°C</p> 
        </div>
        <div class="title">FAN pwm: 
            <p id="intext">{{this.pwm}}</p>
        </div>

        <div class="bbtn"><v-btn :disabled="btnDisable" @click="clk">Refresh</v-btn></div>
    </div>

</template>

<script>
// @ is an alias to /src

export default {
    name: 'Home',
    data() {
        return {
            temperature: 0,
            last_update: 0,
            pwm: 0,
            btnDisable: false,
        }
    },
    components: {},
    methods: {
        clk() {
            this.$axios
                .get(`https://wemosd1dp.azurewebsites.net/`)
                .then(result => {
                    this.temperature = result.data.temperature.toFixed(2)
                    this.last_update = result.data.last_update
                    this.pwm = result.data.pwm
                })
                .catch(err => {
                    console.log(err);
                });
            // this.btnDisable = !this.btnDisable
        },

        setfanspeed(){

        }
    },
    created() {
        console.log('=== Home created! === ')
        this.$axios
                .get(`https://wemosd1dp.azurewebsites.net/`)
                .then(result => {
                    this.temperature = result.data.temperature.toFixed(2)
                    this.last_update = result.data.last_update
                    this.pwm = result.data.pwm
                })
                .catch(err => {
                    console.log(err);
                });
    },
    beforeUpdate() {
        console.log('=== Home beforeUpdate! === ')
    },
    updated() {
        console.log('=== Home updated! === ')
    },
}
</script>
<style lang="scss" scoped>
* {
    // border: 1px solid #f00;
    color: #fff;
}

.home {
    width: 100vw;
    height: 100vh;
    position: relative;
    // background-color: white;
    background-image: linear-gradient(0deg, rgb(68, 137, 194) 0%, rgb(18, 99, 136) 100%);
    background-size: cover;
}
.title{
    margin-left: 50px;
    padding-top: 10px;
    color: rgb(255, 255, 255);
}
#intext {
    font-size: 20px;
}
.bbtn{
    border-radius: 5px;
    height: 25px;
    margin-left: 40px;
}
</style>
